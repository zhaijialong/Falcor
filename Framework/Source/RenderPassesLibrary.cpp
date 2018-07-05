/***************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "Framework.h"
#include "RenderPassesLibrary.h"
#include "API/Device.h"

namespace Falcor
{
    static std::unordered_map<std::string, RenderPassDesc> gRenderPassList;
    static std::vector<HMODULE> gLoadedLibraries;

    bool RenderPassLibrary::addLibrary(const char* libraryName)
    {
        auto lib = LoadLibraryA(libraryName);
        if (!lib)
        {
            logWarning("Can't load render pass library " + std::string(libraryName));
            return false;
        }

        auto func = GetProcAddress(lib, kDllFuncName);
        if (!func)
        {
            FreeLibrary(lib);
            logWarning("Error when loading a render pass library (`" + std::string(libraryName) + "'). Can't find `" + std::string(kDllFuncName) + "` function");
            return false;
        }

        gLoadedLibraries.push_back(lib);

        std::vector<RenderPassDesc> descs;
        RenderPassLibraryFunc* getDescs = (RenderPassLibraryFunc*)func;
        getDescs(gpDevice, descs);

        for (const auto& d : descs)
        {
            if (gRenderPassList.find(d.name) != gRenderPassList.end())
            {
                logWarning(std::string("Trying to load a render-pass `") + d.name + "` from library `" + std::string(libraryName) + "`, but a render-pass with the same name already exists. Ignoring the new definition");                
            }
            else
            {
                gRenderPassList[d.name] = d;
            }
        }

        return true;
    }

    const RenderPassDesc* RenderPassLibrary::getRenderPassDesc(const std::string& name)
    {
        if (gRenderPassList.find(name) == gRenderPassList.end())
        {
            logWarning("Can't find a render-pass named `" + name + "`");
            return nullptr;
        }
        return &gRenderPassList[name];
    }

    std::shared_ptr<RenderPass> RenderPassLibrary::createRenderPass(const std::string& name)
    {
        const auto& pDesc = getRenderPassDesc(name);
        return pDesc ? pDesc->func() : nullptr;
    }

    void RenderPassLibrary::unloadAllLibraries()
    {
        for (auto& a : gLoadedLibraries) FreeLibrary(a);
    }
}