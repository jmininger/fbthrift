/*
 * Copyright 2004-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <mstch/mstch.hpp>
#include <unordered_map>

#include <thrift/compiler/generate/t_generator.h>

class mstch_base;
class mstch_generators;

enum ELEMENT_POSITION {
  NONE = 0,
  FIRST = 1,
  LAST = 2,
  FIRST_AND_LAST = 3,
};

struct mstch_cache {};

class enum_value_generator {
 public:
  virtual ~enum_value_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_enum_value const* enum_value,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class mstch_generators {
 public:
  mstch_generators()
      : enum_value_generator_(std::make_unique<enum_value_generator>()) {}
  ~mstch_generators() = default;

  void set_enum_value_generator(std::unique_ptr<enum_value_generator> g) {
    enum_value_generator_ = std::move(g);
  }

 private:
  std::unique_ptr<enum_value_generator> enum_value_generator_;
};

class mstch_base : public mstch::object {
 public:
  mstch_base(
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION const pos)
      : generators_(generators), cache_(cache), pos_(pos) {
    register_methods(
        this,
        {
            {"first?", &mstch_base::first}, {"last?", &mstch_base::last},
        });
  }
  mstch::node first() {
    return pos_ == ELEMENT_POSITION::FIRST ||
        pos_ == ELEMENT_POSITION::FIRST_AND_LAST;
  }
  mstch::node last() {
    return pos_ == ELEMENT_POSITION::LAST ||
        pos_ == ELEMENT_POSITION::FIRST_AND_LAST;
  }

  static t_type const* resolve_typedef(t_type const* type) {
    while (type->is_typedef()) {
      type = dynamic_cast<t_typedef const*>(type)->get_type();
    }
    return type;
  }

 protected:
  std::shared_ptr<mstch_generators const> generators_;
  std::shared_ptr<mstch_cache> cache_;
  ELEMENT_POSITION const pos_;
};

class mstch_enum_value : public mstch_base {
 public:
  using node_type = t_enum_value;
  mstch_enum_value(
      t_enum_value const* enm_value,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), enm_value_(enm_value) {
    register_methods(
        this,
        {
            {"enumValue:name", &mstch_enum_value::name},
            {"enumValue:value", &mstch_enum_value::value},
        });
  }
  mstch::node name() {
    return enm_value_->get_name();
  }
  mstch::node value() {
    return std::to_string(enm_value_->get_value());
  }

 protected:
  t_enum_value const* enm_value_;
};