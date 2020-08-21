//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsrender.h"
#include "cjsgui.h"
#include <render\Direct2D.h>

namespace clib {


#pragma region image

    cjsrender_image::~cjsrender_image()
    {
        renderer->destroy2();
    }

    std::string cjsrender_image::get_name()
    {
        return "image";
    }

    int cjsrender_image::get_type()
    {
        return r_image;
    }

    void cjsrender_image::set_data(const std::vector<char>& value)
    {
        data = value;
        if (renderer)
        {
            renderer->destroy2();
            if (!data.empty())
                renderer->on_changed();
        }
    }

    const char* cjsrender_image::get_data() const
    {
        return data.data();
    }

    size_t cjsrender_image::get_data_len() const
    {
        return data.size();
    }

    void cjsrender_image_renderer::render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
    {
        auto e = element.lock();
        HRESULT hr = S_OK;

        if (m_cFrames == 1) {
            if (bitmap)
            {
                r->DrawBitmap(
                    bitmap,
                    D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                    1.0f,
                    D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                );
            }
        }

        // Check to see if the render targets are initialized
        else if (m_pFrameComposeRT)
        {
            using namespace std::chrono_literals;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - timer) * 2 >= m_uFrameDelay * 1ms) {
                hr = ComposeNextFrame();
                timer = std::chrono::system_clock::now();
            }
            if (SUCCEEDED(hr))
            {
                D2D1_RECT_F drawRect;
                hr = CalculateDrawRectangle(drawRect, CRect(CPoint(), bounds.Size()));

                if (SUCCEEDED(hr))
                {
                    // Get the bitmap to draw on the hwnd render target
                    bitmap.Release();
                    hr = m_pFrameComposeRT->GetBitmap(&bitmap);
                }

                if (SUCCEEDED(hr))
                {
                    // Draw the bitmap onto the calculated rectangle
                    r->DrawBitmap(
                        bitmap,
                        D2D1::RectF(
                            __max(bounds.left + drawRect.left, (FLOAT)bounds.left),
                            __max(bounds.top + drawRect.top, (FLOAT)bounds.top),
                            __min(bounds.left + drawRect.right, (FLOAT)bounds.right),
                            __min(bounds.top + drawRect.bottom, (FLOAT)bounds.bottom)),
                        1.0f,
                        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                    );
                }
            }
        }
    }

    void cjsrender_image_renderer::init2()
    {
        if (GLOBAL_STATE.canvas.lock())
        {
            auto e = element.lock();
            auto data = e->get_data();
            auto len = e->get_data_len();
            if (data && len)
            {
                // Reset the states
                timer = std::chrono::system_clock::now();
                m_uNextFrameIndex = 0;
                m_uFrameDisposal = DM_NONE;  // No previous frame, use disposal none
                m_uLoopNumber = 0;
                m_fHasLoop = false;
                m_pSavedFrame.Release();

                CComPtr<IStream> stream = SHCreateMemStream((const BYTE*)data, len);
                if (!stream)
                {
                    ATLVERIFY(!"SHCreateMemStream failed");
                }
                // Create a decoder for the gif file
                m_pDecoder.Release();
                auto hr = Direct2D::Singleton().GetWICImagingFactory()->CreateDecoderFromStream(
                    stream,
                    nullptr,
                    WICDecodeMetadataCacheOnLoad,
                    &m_pDecoder);
                if (SUCCEEDED(hr))
                {
                    hr = GetGlobalMetadata();
                }

                auto rt = GLOBAL_STATE.canvas.lock();
                if (m_cFrames == 1) {
                    auto wic = rt->CreateImageFromMemory((LPVOID)data, (int)len);
                    if (wic)
                        bitmap = rt->GetBitmapFromWIC(wic);
                }
                else {
                    if (SUCCEEDED(hr))
                    {
                        hr = rt->GetDirect2DRenderTarget()->CreateCompatibleRenderTarget(
                            D2D1::SizeF(
                                static_cast<float>(m_cxGifImage),
                                static_cast<float>(m_cyGifImage)),
                            &m_pFrameComposeRT);
                    }

                    if (SUCCEEDED(hr))
                    {
                        // If we have at least one frame, start playing
                        // the animation from the first frame
                        if (m_cFrames > 0)
                        {
                            hr = ComposeNextFrame();
                        }
                    }
                }
            }
        }
    }

    void cjsrender_image_renderer::destroy2()
    {
        bitmap = nullptr;
        if (m_pRawFrame)
            m_pRawFrame = nullptr;
        if (m_pSavedFrame)
            m_pSavedFrame = nullptr;
        if (m_pDecoder)
            m_pDecoder = nullptr;
        if (m_pFrameComposeRT)
            m_pFrameComposeRT = nullptr;
    }


    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::GetGlobalMetadata()                 *
    *                                                                 *
    *  Retrieves global metadata which pertains to the entire image.  *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::GetGlobalMetadata()
    {
        PROPVARIANT propValue;
        PropVariantInit(&propValue);
        CComPtr<IWICMetadataQueryReader> pMetadataQueryReader;

        // Get the frame count
        HRESULT hr = m_pDecoder->GetFrameCount(&m_cFrames);
        if (SUCCEEDED(hr))
        {
            // Create a MetadataQueryReader from the decoder
            hr = m_pDecoder->GetMetadataQueryReader(
                &pMetadataQueryReader);
        }

        if (SUCCEEDED(hr))
        {
            // Get background color
            if (FAILED(GetBackgroundColor(pMetadataQueryReader)))
            {
                // Default to transparent if failed to get the color
                m_backgroundColor = D2D1::ColorF(0, 0.f);
            }
        }

        // Get global frame size
        if (SUCCEEDED(hr))
        {
            // Get width
            hr = pMetadataQueryReader->GetMetadataByName(
                L"/logscrdesc/Width",
                &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_cxGifImage = propValue.uiVal;
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Get height
            hr = pMetadataQueryReader->GetMetadataByName(
                L"/logscrdesc/Height",
                &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_cyGifImage = propValue.uiVal;
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Get pixel aspect ratio
            hr = pMetadataQueryReader->GetMetadataByName(
                L"/logscrdesc/PixelAspectRatio",
                &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI1 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    UINT uPixelAspRatio = propValue.bVal;

                    if (uPixelAspRatio != 0)
                    {
                        // Need to calculate the ratio. The value in uPixelAspRatio 
                        // allows specifying widest pixel 4:1 to the tallest pixel of 
                        // 1:4 in increments of 1/64th
                        float pixelAspRatio = (uPixelAspRatio + 15.f) / 64.f;

                        // Calculate the image width and height in pixel based on the
                        // pixel aspect ratio. Only shrink the image.
                        if (pixelAspRatio > 1.f)
                        {
                            m_cxGifImagePixel = m_cxGifImage;
                            m_cyGifImagePixel = static_cast<unsigned int>(m_cyGifImage / pixelAspRatio);
                        }
                        else
                        {
                            m_cxGifImagePixel = static_cast<unsigned int>(m_cxGifImage * pixelAspRatio);
                            m_cyGifImagePixel = m_cyGifImage;
                        }
                    }
                    else
                    {
                        // The value is 0, so its ratio is 1
                        m_cxGifImagePixel = m_cxGifImage;
                        m_cyGifImagePixel = m_cyGifImage;
                    }
                }
                PropVariantClear(&propValue);
            }
        }

        // Get looping information
        if (SUCCEEDED(hr))
        {
            // First check to see if the application block in the Application Extension
            // contains "NETSCAPE2.0" and "ANIMEXTS1.0", which indicates the gif animation
            // has looping information associated with it.
            // 
            // If we fail to get the looping information, loop the animation infinitely.
            if (SUCCEEDED(pMetadataQueryReader->GetMetadataByName(
                L"/appext/application",
                &propValue)) &&
                propValue.vt == (VT_UI1 | VT_VECTOR) &&
                propValue.caub.cElems == 11 &&  // Length of the application block
                (!memcmp(propValue.caub.pElems, "NETSCAPE2.0", propValue.caub.cElems) ||
                    !memcmp(propValue.caub.pElems, "ANIMEXTS1.0", propValue.caub.cElems)))
            {
                PropVariantClear(&propValue);

                hr = pMetadataQueryReader->GetMetadataByName(L"/appext/data", &propValue);
                if (SUCCEEDED(hr))
                {
                    //  The data is in the following format:
                    //  byte 0: extsize (must be > 1)
                    //  byte 1: loopType (1 == animated gif)
                    //  byte 2: loop count (least significant byte)
                    //  byte 3: loop count (most significant byte)
                    //  byte 4: set to zero
                    if (propValue.vt == (VT_UI1 | VT_VECTOR) &&
                        propValue.caub.cElems >= 4 &&
                        propValue.caub.pElems[0] > 0 &&
                        propValue.caub.pElems[1] == 1)
                    {
                        m_uTotalLoopCount = MAKEWORD(propValue.caub.pElems[2],
                            propValue.caub.pElems[3]);

                        // If the total loop count is not zero, we then have a loop count
                        // If it is 0, then we repeat infinitely
                        if (m_uTotalLoopCount != 0)
                        {
                            m_fHasLoop = true;
                        }
                    }
                }
            }
        }

        PropVariantClear(&propValue);
        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::GetRawFrame()                       *
    *                                                                 *
    *  Decodes the current raw frame, retrieves its timing            *
    *  information, disposal method, and frame dimension for          *
    *  rendering.  Raw frame is the frame read directly from the gif  *
    *  file without composing.                                        *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::GetRawFrame(UINT uFrameIndex)
    {
        CComPtr<IWICFormatConverter> pConverter;
        CComPtr<IWICBitmapFrameDecode> pWicFrame;
        CComPtr<IWICMetadataQueryReader> pFrameMetadataQueryReader;

        PROPVARIANT propValue;
        PropVariantInit(&propValue);

        // Retrieve the current frame
        HRESULT hr = m_pDecoder->GetFrame(uFrameIndex, &pWicFrame);
        if (SUCCEEDED(hr))
        {
            // Format convert to 32bppPBGRA which D2D expects
            hr = Direct2D::Singleton().GetWICImagingFactory()->CreateFormatConverter(&pConverter);
        }

        if (SUCCEEDED(hr))
        {
            hr = pConverter->Initialize(
                pWicFrame,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.f,
                WICBitmapPaletteTypeCustom);
        }

        if (SUCCEEDED(hr))
        {
            // Create a D2DBitmap from IWICBitmapSource
            m_pRawFrame.Release();
            hr = GLOBAL_STATE.canvas.lock()->GetDirect2DRenderTarget()->CreateBitmapFromWicBitmap(
                pConverter,
                nullptr,
                &m_pRawFrame);
        }

        if (SUCCEEDED(hr))
        {
            // Get Metadata Query Reader from the frame
            hr = pWicFrame->GetMetadataQueryReader(&pFrameMetadataQueryReader);
        }

        // Get the Metadata for the current frame
        if (SUCCEEDED(hr))
        {
            hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Left", &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_framePosition.left = static_cast<float>(propValue.uiVal);
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Top", &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_framePosition.top = static_cast<float>(propValue.uiVal);
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Width", &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_framePosition.right = static_cast<float>(propValue.uiVal)
                        + m_framePosition.left;
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Height", &propValue);
            if (SUCCEEDED(hr))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    m_framePosition.bottom = static_cast<float>(propValue.uiVal)
                        + m_framePosition.top;
                }
                PropVariantClear(&propValue);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Get delay from the optional Graphic Control Extension
            if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
                L"/grctlext/Delay",
                &propValue)))
            {
                hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
                if (SUCCEEDED(hr))
                {
                    // Convert the delay retrieved in 10 ms units to a delay in 1 ms units
                    hr = UIntMult(propValue.uiVal, 10, &m_uFrameDelay);
                }
                PropVariantClear(&propValue);
            }
            else
            {
                // Failed to get delay from graphic control extension. Possibly a
                // single frame image (non-animated gif)
                m_uFrameDelay = 0;
            }

            if (SUCCEEDED(hr))
            {
                // Insert an artificial delay to ensure rendering for gif with very small
                // or 0 delay.  This delay number is picked to match with most browsers' 
                // gif display speed.
                //
                // This will defeat the purpose of using zero delay intermediate frames in 
                // order to preserve compatibility. If this is removed, the zero delay 
                // intermediate frames will not be visible.
                if (m_uFrameDelay < 90)
                {
                    m_uFrameDelay = 90;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
                L"/grctlext/Disposal",
                &propValue)))
            {
                hr = (propValue.vt == VT_UI1) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    m_uFrameDisposal = propValue.bVal;
                }
            }
            else
            {
                // Failed to get the disposal method, use default. Possibly a 
                // non-animated gif.
                m_uFrameDisposal = DM_UNDEFINED;
            }
        }

        PropVariantClear(&propValue);

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::GetBackgroundColor()                *
    *                                                                 *
    *  Reads and stores the background color for gif.                 *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::GetBackgroundColor(
        CComPtr<IWICMetadataQueryReader> pMetadataQueryReader)
    {
        DWORD dwBGColor;
        BYTE backgroundIndex = 0;
        WICColor rgColors[256];
        UINT cColorsCopied = 0;
        PROPVARIANT propVariant;
        PropVariantInit(&propVariant);
        CComPtr<IWICPalette> pWicPalette;

        // If we have a global palette, get the palette and background color
        HRESULT hr = pMetadataQueryReader->GetMetadataByName(
            L"/logscrdesc/GlobalColorTableFlag",
            &propVariant);
        if (SUCCEEDED(hr))
        {
            hr = (propVariant.vt != VT_BOOL || !propVariant.boolVal) ? E_FAIL : S_OK;
            PropVariantClear(&propVariant);
        }

        if (SUCCEEDED(hr))
        {
            // Background color index
            hr = pMetadataQueryReader->GetMetadataByName(
                L"/logscrdesc/BackgroundColorIndex",
                &propVariant);
            if (SUCCEEDED(hr))
            {
                hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
                if (SUCCEEDED(hr))
                {
                    backgroundIndex = propVariant.bVal;
                }
                PropVariantClear(&propVariant);
            }
        }

        // Get the color from the palette
        if (SUCCEEDED(hr))
        {
            hr = Direct2D::Singleton().GetWICImagingFactory()->CreatePalette(&pWicPalette);
        }

        if (SUCCEEDED(hr))
        {
            // Get the global palette
            hr = m_pDecoder->CopyPalette(pWicPalette);
        }

        if (SUCCEEDED(hr))
        {
            hr = pWicPalette->GetColors(
                ARRAYSIZE(rgColors),
                rgColors,
                &cColorsCopied);
        }

        if (SUCCEEDED(hr))
        {
            // Check whether background color is outside range 
            hr = (backgroundIndex >= cColorsCopied) ? E_FAIL : S_OK;
        }

        if (SUCCEEDED(hr))
        {
            // Get the color in ARGB format
            dwBGColor = rgColors[backgroundIndex];

            // The background color is in ARGB format, and we want to 
            // extract the alpha value and convert it to float
            float alpha = (dwBGColor >> 24) / 255.f;
            m_backgroundColor = D2D1::ColorF(dwBGColor, alpha);
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::CalculateDrawRectangle()            *
    *                                                                 *
    *  Calculates a specific rectangular area of the hwnd             *
    *  render target to draw a bitmap containing the current          *
    *  composed frame.                                                *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::CalculateDrawRectangle(D2D1_RECT_F& drawRect, const CRect& rcClient)
    {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr))
        {
            // Calculate the area to display the image
            // Center the image if the client rectangle is larger
            drawRect.left = (static_cast<float>(rcClient.right) - m_cxGifImagePixel) / 2.f;
            drawRect.top = (static_cast<float>(rcClient.bottom) - m_cyGifImagePixel) / 2.f;
            drawRect.right = drawRect.left + m_cxGifImagePixel;
            drawRect.bottom = drawRect.top + m_cyGifImagePixel;

            // If the client area is resized to be smaller than the image size, scale
            // the image, and preserve the aspect ratio
            auto aspectRatio = static_cast<float>(m_cxGifImagePixel) /
                static_cast<float>(m_cyGifImagePixel);

            if (drawRect.left < 0)
            {
                auto newWidth = static_cast<float>(rcClient.right);
                float newHeight = newWidth / aspectRatio;
                drawRect.left = 0;
                drawRect.top = (static_cast<float>(rcClient.bottom) - newHeight) / 2.f;
                drawRect.right = newWidth;
                drawRect.bottom = drawRect.top + newHeight;
            }

            if (drawRect.top < 0)
            {
                auto newHeight = static_cast<float>(rcClient.bottom);
                float newWidth = newHeight * aspectRatio;
                drawRect.left = (static_cast<float>(rcClient.right) - newWidth) / 2.f;
                drawRect.top = 0;
                drawRect.right = drawRect.left + newWidth;
                drawRect.bottom = newHeight;
            }
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::RestoreSavedFrame()                 *
    *                                                                 *
    *  Copys the saved frame to the frame in the bitmap render        *
    *  target.                                                        *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::RestoreSavedFrame()
    {
        HRESULT hr = S_OK;

        CComPtr<ID2D1Bitmap> pFrameToCopyTo;

        hr = m_pSavedFrame ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            hr = m_pFrameComposeRT->GetBitmap(&pFrameToCopyTo);
        }

        if (SUCCEEDED(hr))
        {
            // Copy the whole bitmap
            hr = pFrameToCopyTo->CopyFromBitmap(nullptr, m_pSavedFrame, nullptr);
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::ClearCurrentFrameArea()             *
    *                                                                 *
    *  Clears a rectangular area equal to the area overlaid by the    *
    *  current raw frame in the bitmap render target with background  *
    *  color.                                                         *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::ClearCurrentFrameArea()
    {
        m_pFrameComposeRT->BeginDraw();

        // Clip the render target to the size of the raw frame
        m_pFrameComposeRT->PushAxisAlignedClip(
            &m_framePosition,
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        m_pFrameComposeRT->Clear(m_backgroundColor);

        // Remove the clipping
        m_pFrameComposeRT->PopAxisAlignedClip();

        return m_pFrameComposeRT->EndDraw();
    }

    /******************************************************************
    *                                                                 *
    *  DemoApp::ComposeNextFrame()                                    *
    *                                                                 *
    *  Composes the next frame by first disposing the current frame   *
    *  and then overlaying the next frame. More than one frame may    *
    *  be processed in order to produce the next frame to be          *
    *  displayed due to the use of zero delay intermediate frames.    *
    *  Also, sets a timer that is equal to the delay of the frame.    *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::ComposeNextFrame()
    {
        HRESULT hr = S_OK;

        // Check to see if the render targets are initialized
        if (m_pFrameComposeRT)
        {
            // First, kill the timer since the delay is no longer valid
            //KillTimer(m_hWnd, DELAY_TIMER_ID);

            // Compose one frame
            hr = DisposeCurrentFrame();
            if (SUCCEEDED(hr))
            {
                hr = OverlayNextFrame();
            }

            // Keep composing frames until we see a frame with delay greater than
            // 0 (0 delay frames are the invisible intermediate frames), or until
            // we have reached the very last frame.
            while (SUCCEEDED(hr) && m_uFrameDelay == 0 && !IsLastFrame())
            {
                hr = DisposeCurrentFrame();
                if (SUCCEEDED(hr))
                {
                    hr = OverlayNextFrame();
                }
            }

            // If we have more frames to play, set the timer according to the delay.
            // Set the timer regardless of whether we succeeded in composing a frame
            // to try our best to continue displaying the animation.
            if (!EndOfAnimation() && m_cFrames > 1)
            {
                // Set the timer according to the delay
                //SetTimer(m_hWnd, DELAY_TIMER_ID, m_uFrameDelay, nullptr);
            }
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::DisposeCurrentFrame()               *
    *                                                                 *
    *  At the end of each delay, disposes the current frame           *
    *  based on the disposal method specified.                        *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::DisposeCurrentFrame()
    {
        HRESULT hr = S_OK;

        switch (m_uFrameDisposal)
        {
        case DM_UNDEFINED:
        case DM_NONE:
            // We simply draw on the previous frames. Do nothing here.
            break;
        case DM_BACKGROUND:
            // Dispose background
            // Clear the area covered by the current raw frame with background color
            hr = ClearCurrentFrameArea();
            break;
        case DM_PREVIOUS:
            // Dispose previous
            // We restore the previous composed frame first
            hr = RestoreSavedFrame();
            break;
        default:
            // Invalid disposal method
            hr = E_FAIL;
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::OverlayNextFrame()                  *
    *                                                                 *
    *  Loads and draws the next raw frame into the composed frame     *
    *  render target. This is called after the current frame is       *
    *  disposed.                                                      *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::OverlayNextFrame()
    {
        // Get Frame information
        HRESULT hr = GetRawFrame(m_uNextFrameIndex);
        if (SUCCEEDED(hr))
        {
            // For disposal 3 method, we would want to save a copy of the current
            // composed frame
            if (m_uFrameDisposal == DM_PREVIOUS)
            {
                hr = SaveComposedFrame();
            }
        }

        if (SUCCEEDED(hr))
        {
            // Start producing the next bitmap
            m_pFrameComposeRT->BeginDraw();

            // If starting a new animation loop
            if (m_uNextFrameIndex == 0)
            {
                // Draw background and increase loop count
                m_pFrameComposeRT->Clear(m_backgroundColor);
                m_uLoopNumber++;
            }

            // Produce the next frame
            m_pFrameComposeRT->DrawBitmap(
                m_pRawFrame,
                m_framePosition);

            hr = m_pFrameComposeRT->EndDraw();
        }

        // To improve performance and avoid decoding/composing this frame in the 
        // following animation loops, the composed frame can be cached here in system 
        // or video memory.

        if (SUCCEEDED(hr))
        {
            // Increase the frame index by 1
            m_uNextFrameIndex = (++m_uNextFrameIndex) % m_cFrames;
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  cjsrender_image_renderer::SaveComposedFrame()                 *
    *                                                                 *
    *  Saves the current composed frame in the bitmap render target   *
    *  into a temporary bitmap. Initializes the temporary bitmap if   *
    *  needed.                                                        *
    *                                                                 *
    ******************************************************************/

    HRESULT cjsrender_image_renderer::SaveComposedFrame()
    {
        HRESULT hr = S_OK;

        CComPtr<ID2D1Bitmap> pFrameToBeSaved;

        hr = m_pFrameComposeRT->GetBitmap(&pFrameToBeSaved);
        if (SUCCEEDED(hr))
        {
            // Create the temporary bitmap if it hasn't been created yet 
            if (m_pSavedFrame == nullptr)
            {
                auto bitmapSize = pFrameToBeSaved->GetPixelSize();
                D2D1_BITMAP_PROPERTIES bitmapProp;
                pFrameToBeSaved->GetDpi(&bitmapProp.dpiX, &bitmapProp.dpiY);
                bitmapProp.pixelFormat = pFrameToBeSaved->GetPixelFormat();

                hr = m_pFrameComposeRT->CreateBitmap(
                    bitmapSize,
                    bitmapProp,
                    &m_pSavedFrame);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Copy the whole bitmap
            hr = m_pSavedFrame->CopyFromBitmap(nullptr, pFrameToBeSaved, nullptr);
        }

        return hr;
    }

    bool cjsrender_image_renderer::IsLastFrame() const
    {
        return (m_uNextFrameIndex == 0);
    }

    bool cjsrender_image_renderer::EndOfAnimation() const
    {
        return m_fHasLoop && IsLastFrame() && m_uLoopNumber == m_uTotalLoopCount + 1;
    }

#pragma endregion image
}

