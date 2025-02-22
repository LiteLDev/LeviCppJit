#include "ServerSymbolGenerator.h"

#include "lcj/core/LeviCppJit.h"

#include <ll/api/memory/Memory.h>

namespace lcj {

llvm::Error ServerSymbolGenerator::tryToGenerate(
    llvm::orc::LookupState&           LS,
    llvm::orc::LookupKind             K,
    llvm::orc::JITDylib&              JD,
    llvm::orc::JITDylibLookupFlags    JDLookupFlags,
    llvm::orc::SymbolLookupSet const& Symbols
) {
    llvm::orc::SymbolMap newSymbols;

    bool hasGlobalPrefix = (globalPrefix != '\0');

    for (auto& KV : Symbols) {
        std::string_view name = *KV.first;
        if (name.empty()) continue;
        if (hasGlobalPrefix && name.front() != globalPrefix) continue;

        name.remove_prefix(hasGlobalPrefix);

        // LeviCppJit::getInstance().getLogger().debug("resolveSymbol: {}", name);

        if (void* addr = ll::memory::resolveSymbol(name, true))
            newSymbols[KV.first] = llvm::JITEvaluatedSymbol{
                static_cast<llvm::JITTargetAddress>(reinterpret_cast<uintptr_t>(addr)),
                llvm::JITSymbolFlags::Exported
            };
    }

    if (newSymbols.empty()) return llvm::Error::success();

    return JD.define(absoluteSymbols(std::move(newSymbols)));
}

} // namespace lcj