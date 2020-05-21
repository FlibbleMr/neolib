// ecs.cpp
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

#include <neolib/neolib.hpp>
#include <neolib/app/i_power.hpp>
#include <neolib/ecs/ecs.hpp>
#include <neolib/ecs/entity_info.hpp>
#include <neolib/ecs/time.hpp>
#include <neolib/core/numerical.hpp>

namespace neolib::ecs
{
    const ecs::archetype_registry_t& ecs::archetypes() const
    {
        return iArchetypeRegistry;
    }

    ecs::archetype_registry_t& ecs::archetypes()
    {
        return iArchetypeRegistry;
    }

    const ecs::component_factories_t& ecs::component_factories() const
    {
        return iComponentFactories;
    }

    ecs::component_factories_t& ecs::component_factories()
    {
        return iComponentFactories;
    }

    const ecs::components_t& ecs::components() const
    {
        return iComponents;
    }

    ecs::components_t& ecs::components()
    {
        return iComponents;
    }

    const ecs::shared_component_factories_t& ecs::shared_component_factories() const
    {
        return iSharedComponentFactories;
    }

    ecs::shared_component_factories_t& ecs::shared_component_factories()
    {
        return iSharedComponentFactories;
    }

    const ecs::shared_components_t& ecs::shared_components() const
    {
        return iSharedComponents;
    }

    ecs::shared_components_t& ecs::shared_components()
    {
        return iSharedComponents;
    }

    const ecs::system_factories_t& ecs::system_factories() const
    {
        return iSystemFactories;
    }

    ecs::system_factories_t& ecs::system_factories()
    {
        return iSystemFactories;
    }

    const ecs::systems_t& ecs::systems() const
    {
        return iSystems;
    }

    ecs::systems_t& ecs::systems()
    {
        return iSystems;
    }

    const i_entity_archetype& ecs::archetype(entity_archetype_id aArchetypeId) const
    {
        auto existingArchetype = archetypes().find(aArchetypeId);
        if (existingArchetype != archetypes().end())
            return *existingArchetype->second;
        throw entity_archetype_not_found();
    }

    i_entity_archetype& ecs::archetype(entity_archetype_id aArchetypeId)
    {
        return const_cast<i_entity_archetype&>(to_const(*this).archetype(aArchetypeId));
    }

    bool ecs::component_instantiated(component_id aComponentId) const
    {
        return components().find(aComponentId) != components().end();
    }

    const i_component& ecs::component(component_id aComponentId) const
    {
        auto existingComponent = components().find(aComponentId);
        if (existingComponent != components().end())
            return *existingComponent->second;
        auto existingFactory = component_factories().find(aComponentId);
        if (existingFactory != component_factories().end())
            return *iComponents.emplace(aComponentId, existingFactory->second()).first->second;
        throw component_not_found();
    }

    i_component& ecs::component(component_id aComponentId)
    {
        return const_cast<i_component&>(to_const(*this).component(aComponentId));
    }

    bool ecs::shared_component_instantiated(component_id aComponentId) const
    {
        return shared_components().find(aComponentId) != shared_components().end();
    }

    const i_shared_component& ecs::shared_component(component_id aComponentId) const
    {
        auto existingComponent = shared_components().find(aComponentId);
        if (existingComponent != shared_components().end())
            return *existingComponent->second;
        auto existingFactory = shared_component_factories().find(aComponentId);
        if (existingFactory != shared_component_factories().end())
            return *iSharedComponents.emplace(aComponentId, existingFactory->second()).first->second;
        throw component_not_found();
    }

    i_shared_component& ecs::shared_component(component_id aComponentId)
    {
        return const_cast<i_shared_component&>(to_const(*this).shared_component(aComponentId));
    }

    bool ecs::system_instantiated(system_id aSystemId) const
    {
        return systems().find(aSystemId) != systems().end();
    }

    const i_system& ecs::system(system_id aSystemId) const
    {
        auto existingSystem = systems().find(aSystemId);
        if (existingSystem != systems().end())
            return *existingSystem->second;
        auto existingFactory = system_factories().find(aSystemId);
        if (existingFactory != system_factories().end())
        {
            auto& newSystem = *iSystems.emplace(aSystemId, existingFactory->second()).first->second;
            if (all_systems_paused())
                newSystem.pause();
            return newSystem;
        }
        throw system_not_found();
    }

    i_system& ecs::system(system_id aSystemId)
    {
        return const_cast<i_system&>(to_const(*this).system(aSystemId));
    }

    entity_id ecs::next_entity_id()
    {
        if (!iFreedEntityIds.empty())
        {
            auto nextId = iFreedEntityIds.back();
            iFreedEntityIds.pop_back();
            return nextId;
        }
        if (++iNextEntityId == null_entity)
            throw entity_ids_exhausted();
        return iNextEntityId;
    }

    void ecs::free_entity_id(entity_id aId)
    {
        iFreedEntityIds.push_back(aId);
    }

    ecs::ecs(ecs_flags aCreationFlags) :
        iFlags{ aCreationFlags }, iNextEntityId { null_entity }, iNextHandleId{ null_id },
        iSystemTimer
        {
            service<async_task>(),
            [this](neolib::callback_timer& aTimer)
            {
                aTimer.again();
                for (auto& system : systems())
                    system.second->apply();
            }, 1, true
        },
        iSystemsPaused{ (flags() & ecs_flags::CreatePaused) == ecs_flags::CreatePaused }
    {
        if ((flags() & ecs_flags::PopulateEntityInfo) == ecs_flags::PopulateEntityInfo)
        {
            register_component<entity_info>();
            register_system<time>();
        }

        if ((flags() & ecs_flags::Turbo) == ecs_flags::Turbo && !all_systems_paused())
            service<i_power>().enable_turbo_mode();

        set_alive();
    }

    ecs::~ecs()
    {
        if (iThreadPool)
            iThreadPool->stop();
        for (auto& system : systems())
            system.second->terminate();
    }

    neolib::recursive_spinlock& ecs::mutex() const
    {
        return iMutex;
    }

    neolib::thread_pool& ecs::thread_pool() const
    {
        std::scoped_lock<neolib::i_lockable> lock{ mutex() };
        if (!iThreadPool)
            iThreadPool.emplace();
        return *iThreadPool;
    }

    ecs_flags ecs::flags() const
    {
        return iFlags;
    }

    entity_id ecs::create_entity(const entity_archetype_id& aArchetypeId)
    {
        auto entityId = next_entity_id();
        if ((flags() & ecs_flags::PopulateEntityInfo) == ecs_flags::PopulateEntityInfo)
            component<entity_info>().populate(entityId, entity_info{ aArchetypeId, system<time>().world_time() });
        EntityCreated.trigger(entityId);
        return entityId;
    }
    
    void ecs::destroy_entity(entity_id aEntityId, bool aNotify)
    {
        if (aNotify)
            EntityDestroyed.trigger(aEntityId);
        for (auto& component : iComponents)
            if (component.second->has_entity_record(aEntityId))
                component.second->destroy_entity_record(aEntityId);
        free_entity_id(aEntityId);
    }

    bool ecs::all_systems_paused() const
    {
        return iSystemsPaused;
    }

    void ecs::pause_all_systems()
    {
        if (iSystemsPaused)
            return;
        for (auto& s : systems())
            s.second->pause();
        iSystemsPaused = true;
        SystemsPaused.trigger();

        if ((flags() & ecs_flags::Turbo) == ecs_flags::Turbo)
            service<i_power>().enable_green_mode();
    }

    void ecs::resume_all_systems()
    {
        if (!iSystemsPaused)
            return;
        for (auto& s : systems())
            s.second->resume();
        iSystemsPaused = false;
        SystemsResumed.trigger();

        if ((flags() & ecs_flags::Turbo) == ecs_flags::Turbo)
            service<i_power>().enable_turbo_mode();
    }

    bool ecs::archetype_registered(const i_entity_archetype& aArchetype) const
    {
        return archetypes().find(aArchetype.id()) != archetypes().end();
    }

    void ecs::register_archetype(const i_entity_archetype& aArchetype)
    {
        if (!archetypes().emplace(aArchetype.id(), std::shared_ptr<const i_entity_archetype>{ std::shared_ptr<const i_entity_archetype>{}, &aArchetype}).second)
            throw uuid_exists("register_archetype");
    }

    void ecs::register_archetype(std::shared_ptr<const i_entity_archetype> aArchetype)
    {
        if (!archetypes().emplace(aArchetype->id(), aArchetype).second)
            throw uuid_exists("register_archetype");
    }

    bool ecs::component_registered(component_id aComponentId) const
    {
        return component_factories().find(aComponentId) != component_factories().end();
    }

    void ecs::register_component(component_id aComponentId, component_factory aFactory)
    {
        if (!component_factories().emplace(aComponentId, aFactory).second)
            throw uuid_exists("register_component");
    }

    bool ecs::shared_component_registered(component_id aComponentId) const
    {
        return shared_component_factories().find(aComponentId) != shared_component_factories().end();
    }

    void ecs::register_shared_component(component_id aComponentId, shared_component_factory aFactory)
    {
        if (!shared_component_factories().emplace(aComponentId, aFactory).second)
            throw uuid_exists("register_shared_component");
    }

    bool ecs::system_registered(system_id aSystemId) const
    {
        return system_factories().find(aSystemId) != system_factories().end();
    }

    void ecs::register_system(system_id aSystemId, system_factory aFactory)
    {
        if (!system_factories().emplace(aSystemId, aFactory).second)
            throw uuid_exists("register_system");
    }

    ecs::handle_t ecs::to_handle(handle_id aId) const
    {
        return iHandles[aId];
    }

    handle_id ecs::add_handle(const std::type_info&, handle_t aHandle)
    {
        auto nextHandleId = next_handle_id();
        if (iHandles.size() <= nextHandleId)
            iHandles.resize(nextHandleId);
        iHandles[nextHandleId] = aHandle;
        return nextHandleId;
    }

    ecs::handle_t ecs::update_handle(handle_id aId, const std::type_info&, handle_t aHandle)
    {
        if (iHandles.size() <= aId)
            throw invalid_handle_id();
        iHandles[aId] = aHandle;
        return aHandle;
    }

    ecs::handle_t ecs::release_handle(handle_id aId)
    {
        if (iHandles.size() <= aId)
            throw invalid_handle_id();
        auto handle = iHandles[aId];
        iHandles[aId] = nullptr;
        free_handle_id(aId);
        return handle;
    }

    handle_id ecs::next_handle_id()
    {
        if (!iFreedHandleIds.empty())
        {
            auto nextId = iFreedHandleIds.back();
            iFreedHandleIds.pop_back();
            return nextId;
        }
        if (++iNextHandleId == null_id)
            throw handle_ids_exhausted();
        return iNextHandleId;
    }

    void ecs::free_handle_id(handle_id aId)
    {
        iFreedHandleIds.push_back(aId);
    }
}