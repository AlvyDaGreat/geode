#pragma once

#include <Geode/DefaultInclude.hpp>
#include "Types.hpp"
#include <vector>
#include <variant>
#include "../utils/casts.hpp"
#include "../utils/Result.hpp"
#include "Mod.hpp"

namespace geode {
	class Hook;

	/**
	 * For developing your own mods, it is 
	 * often convenient to be able to do things 
	 * like create hooks using statically 
	 * initialized global classes.
	 * 
	 * At that point however, your mod has not 
	 * yet received the Mod* to create hooks 
	 * through.
	 * 
	 * For situations like that, you can instead 
	 * inherit from Interface to create your own 
	 * mod interface and create hooks through that; 
	 * calling `init` with the Mod* you receive 
	 * from geode will automatically create all 
	 * scheduled hooks, logs, etc.
	 * 
	 * Interface also provides a handy & 
	 * standardized way to store access to your 
	 * Mod*; you can just define a `get` function 
	 * and a getter for the Mod* stored in the 
	 * Interface.
	 * 
	 * @class Interface
	 */
	class Interface {
	protected:
		static GEODE_DLL Interface* create();
		

		struct ScheduledHook {
			std::string m_displayName;
			void* m_address;
			Result<Hook*>(Mod::*m_addFunction)(std::string const&, void*);
		};

		using ScheduledFunction = std::function<void()>;

		Mod* m_mod = nullptr;
		std::vector<ScheduledHook> m_scheduledHooks;
		std::vector<ScheduledFunction> m_scheduledFunctions;
		
	public:
		static inline GEODE_HIDDEN Interface* get() {
			static Interface* ret = create();
			return ret;
		}

		[[deprecated("Use Mod::get instead")]]
		static inline GEODE_HIDDEN Mod* mod() {
			return Interface::get()->m_mod;
		}

		GEODE_DLL void init(Mod*);

        /**
         * Create a hook at an address. This function can 
		 * be used at static initialization time, as it 
		 * doesn't require the Mod* to be set -- it will 
		 * create the hooks later when the Mod* is set.
		 * 
         * @param address The absolute address of 
         * the function to hook, i.e. gd_base + 0xXXXX
         * @param detour Pointer to your detour function
		 * 
         * @returns Successful result containing the 
         * Hook handle (or nullptr if Mod* is not loaded 
		 * yet), errorful result with info on error
         */
        template<auto Detour, template <class, class...> class Convention>
        Result<Hook*> addHook(void* address) {
        	return Interface::addHook<Detour, Convention>("", address);
        }

        /**
         * The same as addHook(void*, void*), but also provides 
         * a display name to show it in the list of the loader.
         * Mostly for internal use but if you don't like your
         * hooks showing up like base + 0x123456 it can be useful
         */
        template<auto Detour, template <class, class...> class Convention>
        Result<Hook*> addHook(std::string const& displayName, void* address) {
        	if (this->m_mod) {
        		return this->m_mod->addHook<Detour, Convention>(displayName, address);
        	}
        	this->m_scheduledHooks.push_back({ displayName, address, &Mod::addHook<Detour, Convention> });
			return Ok<Hook*>(nullptr);
        }

        GEODE_DLL void scheduleOnLoad(ScheduledFunction function);

        friend Mod* Mod::get<void>();
	};

	template<>
	inline Mod* Mod::get<void>() {
		return Interface::get()->m_mod;
	}
	
    inline Mod* getMod() {
        return Mod::get();
    }
}

inline const char* operator"" _spr(const char* str, size_t) {
    return geode::Mod::get()->expandSpriteName(str);
}
