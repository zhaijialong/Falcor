#pragma once
#include "Falcor.h"
#include "SampleTest.h"

using namespace Falcor;

class UpscaleFilter : public Renderer
{
public:
	void onLoad(SampleCallbacks* pSample, RenderContext::SharedPtr pContext) override;
	void onFrameRender(SampleCallbacks* pSample, RenderContext::SharedPtr pContext, Fbo::SharedPtr pTargetFbo) override;
	void onShutdown(SampleCallbacks* pSample) override;
	void onGuiRender(SampleCallbacks* pSample, Gui* pGui) override;

private:

	Texture::SharedPtr mpTestImage;
	Sampler::SharedPtr mpPointSampler;
	Sampler::SharedPtr mpLinearSampler;
	FullScreenPass::UniquePtr mpUpscalePass;
	GraphicsVars::SharedPtr mpProgVars;

	enum FilterMode
	{
		Point,
		Bilinear,
		Bicubic
	};

	FilterMode mFilter = Point;
};