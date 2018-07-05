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
#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Falcor
{
    class RenderPass;
    class Device;

    struct RenderPassDesc
    {
        using CreateFunc = std::function<std::shared_ptr<RenderPass>()>;
        const char* name = nullptr;
        const char* desc = nullptr;
        CreateFunc func = nullptr;

        RenderPassDesc() = default;
        RenderPassDesc(const char* name_, const char* desc_, CreateFunc func_) : name(name_), desc(desc_), func(func_) {}
    };

    using RenderPassLibraryFunc = void(std::shared_ptr<Device> pDevice, std::vector<RenderPassDesc>& passesList);
    static const char* kDllFuncName = "getRenderPassList";

    class RenderPassLibrary
    {
    public:
        static bool addLibrary(const char* libraryName);
        static const RenderPassDesc* getRenderPassDesc(const std::string& name);
        static std::shared_ptr<RenderPass> createRenderPass(const std::string& name);
        static void unloadAllLibraries();
    };
}