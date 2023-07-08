#include <SFML/Window.hpp>

#include <igl/IGL.h>
#include <igl/vulkan/Common.h>
#include <igl/vulkan/Device.h>
#include <igl/vulkan/HWDevice.h>
#include <igl/vulkan/PlatformDevice.h>
#include <igl/vulkan/VulkanContext.h>

#include <cassert>
#include <memory>
#include <string>

static const std::string vertexShaderCode = R"(
#version 460
layout (location=0) out vec3 color;
const vec2 pos[3] = vec2[3](
	vec2(-0.6, -0.4),
	vec2( 0.6, -0.4),
	vec2( 0.0,  0.6)
);
const vec3 col[3] = vec3[3](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);
void main() {
	gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
	color = col[gl_VertexIndex];
}
)";

static const std::string fragmentShaderCode = R"(
#version 460
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main() {
	out_FragColor = vec4(color, 1.0);
};
)";

class TriangleExample
{
private:
    static const uint32_t KNumColorAttachments = 1;

    sf::WindowBase m_window;
    std::unique_ptr<igl::IDevice> m_device;
    std::shared_ptr<igl::ICommandQueue> m_commandQueue;
    igl::RenderPassDesc m_renderPass;
    std::shared_ptr<igl::IFramebuffer> m_framebuffer;
    std::shared_ptr<igl::IRenderPipelineState> m_renderPipelineState_Triangle;

public:
    TriangleExample();
    ~TriangleExample();

    void run();

private:
    void initializeIGL();
    void createRenderPipeline();
    std::shared_ptr<igl::ITexture> getNativeDrawable();
    void createFramebuffer(const std::shared_ptr<igl::ITexture> &nativeDrawable);
    void render(const std::shared_ptr<igl::ITexture> &nativeDrawable);
};

TriangleExample::TriangleExample()
    : m_window{sf::VideoMode{{800u, 600u}}, "SFML & IGL - Vulkan Triangle", sf::Style::Resize | sf::Style::Close}
{
    initializeIGL();
    createFramebuffer(getNativeDrawable());
    createRenderPipeline();
}

TriangleExample::~TriangleExample()
{
    m_renderPipelineState_Triangle = nullptr;
    m_framebuffer = nullptr;
    m_device.reset(nullptr);
}

void TriangleExample::run()
{
    while (m_window.isOpen())
    {
        render(getNativeDrawable());

        for (auto event = sf::Event{}; m_window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.scancode == sf::Keyboard::Scan::Escape))
            {
                m_window.close();
            }
            else if (event.type == sf::Event::Resized)
            {
                auto *vulkanDevice = static_cast<igl::vulkan::Device *>(m_device.get());
                auto &ctx = vulkanDevice->getVulkanContext();
                ctx.initSwapchain(event.size.width, event.size.height);
            }
        }
    }
}

void TriangleExample::initializeIGL()
{
    // create a device
    {
        const auto cfg = igl::vulkan::VulkanContextConfig{
            .maxTextures = 8,
            .maxSamplers = 8,
            .terminateOnValidationError = true,
            .swapChainColorSpace = igl::ColorSpace::SRGB_LINEAR,
        };

        auto ctx = igl::vulkan::HWDevice::createContext(cfg, m_window.getSystemHandle());

        auto devices = igl::vulkan::HWDevice::queryDevices(*ctx.get(), igl::HWDeviceQueryDesc(igl::HWDeviceType::DiscreteGpu), nullptr);
        if (devices.empty())
        {
            devices = igl::vulkan::HWDevice::queryDevices(*ctx.get(), igl::HWDeviceQueryDesc(igl::HWDeviceType::IntegratedGpu), nullptr);
        }
        m_device = igl::vulkan::HWDevice::create(std::move(ctx), devices[0], (uint32_t)m_window.getSize().x, (uint32_t)m_window.getSize().y);
        assert(m_device && "No GPU detected");
    }

    // Command queue: backed by different types of GPU HW queues
    auto desc = igl::CommandQueueDesc{igl::CommandQueueType::Graphics};
    m_commandQueue = m_device->createCommandQueue(desc, nullptr);

    m_renderPass.colorAttachments.resize(KNumColorAttachments);

    // first color attachment
    for (auto i = 0; i < KNumColorAttachments; ++i)
    {
        // Generate sparse color attachments by skipping alternate slots
        if (i & 0x1)
        {
            continue;
        }
        m_renderPass.colorAttachments[i] = igl::RenderPassDesc::ColorAttachmentDesc{};
        m_renderPass.colorAttachments[i].loadAction = igl::LoadAction::Clear;
        m_renderPass.colorAttachments[i].storeAction = igl::StoreAction::Store;
        m_renderPass.colorAttachments[i].clearColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }
    m_renderPass.depthAttachment.loadAction = igl::LoadAction::DontCare;
}

void TriangleExample::createRenderPipeline()
{
    if (m_renderPipelineState_Triangle)
    {
        return;
    }

    assert(m_framebuffer);

    auto desc = igl::RenderPipelineDesc{};

    desc.targetDesc.colorAttachments.resize(KNumColorAttachments);

    for (auto i = 0; i < KNumColorAttachments; ++i)
    {
        // @fb-only
        if (m_framebuffer->getColorAttachment(i))
        {
            desc.targetDesc.colorAttachments[i].textureFormat = m_framebuffer->getColorAttachment(i)->getFormat();
        }
    }

    if (m_framebuffer->getDepthAttachment())
    {
        desc.targetDesc.depthAttachmentFormat = m_framebuffer->getDepthAttachment()->getFormat();
    }

    desc.shaderStages = igl::ShaderStagesCreator::fromModuleStringInput(*m_device, vertexShaderCode.c_str(), "main", "", fragmentShaderCode.c_str(), "main", "", nullptr);
    m_renderPipelineState_Triangle = m_device->createRenderPipeline(desc, nullptr);
    assert(m_renderPipelineState_Triangle);
}

std::shared_ptr<igl::ITexture> TriangleExample::getNativeDrawable()
{
    igl::Result ret;
    std::shared_ptr<igl::ITexture> drawable;

    const auto &platformDevice = m_device->getPlatformDevice<igl::vulkan::PlatformDevice>();
    assert(platformDevice != nullptr);
    drawable = platformDevice->createTextureFromNativeDrawable(&ret);

    assert(ret.isOk() && ret.message.c_str());
    assert(drawable != nullptr);
    return drawable;
}

void TriangleExample::createFramebuffer(const std::shared_ptr<igl::ITexture> &nativeDrawable)
{
    auto framebufferDesc = igl::FramebufferDesc{};
    framebufferDesc.colorAttachments[0].texture = nativeDrawable;

    for (auto i = 1; i < KNumColorAttachments; ++i)
    {
        // Generate sparse color attachments by skipping alternate slots
        if (i & 0x1)
        {
            continue;
        }
        const igl::TextureDesc desc = igl::TextureDesc::new2D(
            nativeDrawable->getFormat(),
            nativeDrawable->getDimensions().width,
            nativeDrawable->getDimensions().height,
            igl::TextureDesc::TextureUsageBits::Attachment | igl::TextureDesc::TextureUsageBits::Sampled,
            IGL_FORMAT("{}C{}", framebufferDesc.debugName.c_str(), i - 1).c_str());

        framebufferDesc.colorAttachments[i].texture = m_device->createTexture(desc, nullptr);
    }
    m_framebuffer = m_device->createFramebuffer(framebufferDesc, nullptr);
    assert(m_framebuffer);
}

void TriangleExample::render(const std::shared_ptr<igl::ITexture> &nativeDrawable)
{
    const auto size = m_framebuffer->getColorAttachment(0)->getSize();
    if (size.width != m_window.getSize().x || size.height != m_window.getSize().y)
    {
        createFramebuffer(nativeDrawable);
    }
    else
    {
        m_framebuffer->updateDrawable(nativeDrawable);
    }

    // Command buffers (1-N per thread): create, submit and forget
    auto cbDesc = igl::CommandBufferDesc{};
    auto buffer = m_commandQueue->createCommandBuffer(cbDesc, nullptr);

    const auto viewport = igl::Viewport{0.0f, 0.0f, (float)m_window.getSize().x, (float)m_window.getSize().y, 0.0f, +1.0f};
    const auto scissor = igl::ScissorRect{0, 0, (uint32_t)m_window.getSize().x, (uint32_t)m_window.getSize().y};

    // This will clear the framebuffer
    auto commands = buffer->createRenderCommandEncoder(m_renderPass, m_framebuffer);

    commands->bindRenderPipelineState(m_renderPipelineState_Triangle);
    commands->bindViewport(viewport);
    commands->bindScissorRect(scissor);
    commands->pushDebugGroupLabel("Render Triangle", igl::Color(1, 0, 0));
    commands->draw(igl::PrimitiveType::Triangle, 0, 3);
    commands->popDebugGroupLabel();
    commands->endEncoding();

    buffer->present(nativeDrawable);

    m_commandQueue->submit(*buffer);
}

int main()
{
    auto app = TriangleExample{};
    app.run();
}
