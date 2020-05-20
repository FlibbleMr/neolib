// entity_info.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <neolib/neolib.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/numerical.hpp>
#include <neolib/ecs/i_component_data.hpp>
#include <neolib/ecs/ecs_ids.hpp>

namespace neolib::ecs
{
    struct entity_info
    {
        neolib::uuid archetypeId;
        i64 creationTime;
        #ifndef NDEBUG
        bool debug = false;
        #endif

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x867e30c2, 0xaf8e, 0x452e, 0xa542, { 0xd, 0xd0, 0xd1, 0x1, 0xe4, 0x2d } };
                return sId;
            }
            static const neolib::i_string& name()
            {
                static const neolib::string sName = "Entity Info";
                return sName;
            }
            static uint32_t field_count()
            { 
                #ifdef NDEBUG
                return 2;
                #else
                return 3;
                #endif
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Uuid;
                case 1:
                    return component_data_field_type::Int64;
                #ifndef NDEBUG
                case 2:
                    return component_data_field_type::Bool;
                #endif
                default:
                    throw invalid_field_index();
                }
            }
            static const neolib::i_string& field_name(uint32_t aFieldIndex)
            {
                static const neolib::string sFieldNames[] =
                {
                    "Archetype Id",
                    "Creation Time",
                    #ifndef NDEBUG
                    "Debug",
                    #endif
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}