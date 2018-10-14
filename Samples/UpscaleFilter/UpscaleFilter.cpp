#include "UpscaleFilter.h"

void UpscaleFilter::onLoad(SampleCallbacks * pSample, RenderContext::SharedPtr pContext)
{
	auto fboFormat = pSample->getCurrentFbo()->getColorTexture(0)->getFormat();
	mpTestImage = createTextureFromFile("lena512color.tiff", false, isSrgbFormat(fboFormat));

	Sampler::Desc desc;
	mpPointSampler = Sampler::create(desc);

	desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Point);
	mpLinearSampler = Sampler::create(desc);

	mpUpscalePass = FullScreenPass::create("UpscaleFilter.ps.hlsl");
	mpProgVars = GraphicsVars::create(mpUpscalePass->getProgram()->getReflector());
}

void UpscaleFilter::onFrameRender(SampleCallbacks * pSample, RenderContext::SharedPtr pContext, Fbo::SharedPtr pTargetFbo)
{
	mpProgVars->setTexture("gTexture", mpTestImage);
	mpProgVars->setSampler("gSampler", mFilter == FilterMode::Point ? mpPointSampler : mpLinearSampler);
	pContext->setGraphicsVars(mpProgVars);
	
	mpUpscalePass->getProgram()->clearDefines();
	if (mFilter == FilterMode::Bicubic)
	{
		mpUpscalePass->getProgram()->addDefine("BICUBIC");
	}

	mpUpscalePass->execute(pContext.get());
}

void UpscaleFilter::onShutdown(SampleCallbacks * pSample)
{
}

void UpscaleFilter::onGuiRender(SampleCallbacks * pSample, Gui * pGui)
{
	Gui::DropdownList values
	{
		{FilterMode::Point, "Point"},
		{FilterMode::Bilinear, "Bilinear"},
		{FilterMode::Bicubic, "Bicubic"}
	};

	uint32_t var = mFilter;
	pGui->addDropdown("Filter", values, var);
	mFilter = (FilterMode)var;
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
	UpscaleFilter::UniquePtr pRenderer = std::make_unique<UpscaleFilter>();

	SampleConfig config;
	config.windowDesc.title = "Upscale Filter";
	config.windowDesc.width = 1600;
	config.windowDesc.height = 900;
	config.deviceDesc.enableVsync = true;
#ifdef _WIN32
	Sample::run(config, pRenderer);
#else
	config.argc = (uint32_t)argc;
	config.argv = argv;
	Sample::run(config, pRenderer);
#endif
	return 0;
}
