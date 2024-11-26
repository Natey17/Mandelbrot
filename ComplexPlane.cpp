#include "ComplexPlane.h"
#include <cmath>
#include <complex>
#include <sstream>
#include <thread>
#include <vector>

using namespace sf;

ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight) {
    m_pixel_size = { static_cast<unsigned int>(pixelWidth), static_cast<unsigned int>(pixelHeight) };
    m_aspectRatio = static_cast<float>(pixelHeight) / static_cast<float>(pixelWidth);
    m_plane_center = { 0.0f, 0.0f };
    m_plane_size = { BASE_WIDTH, BASE_HEIGHT * m_aspectRatio };
    m_zoomCount = 0;
    m_State = State::CALCULATING;
    m_vArray.setPrimitiveType(Points);
    m_vArray.resize(m_pixel_size.x * m_pixel_size.y);
}

void ComplexPlane::draw(RenderTarget& target, RenderStates states) const {
    target.draw(m_vArray, states);
}

void ComplexPlane::updateRender() {
    if (m_State == State::CALCULATING) {
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 5;
        }
        std::vector<std::thread> threads;
        unsigned int rowsPerThread = m_pixel_size.y / numThreads;

        for (unsigned int t = 0; t < numThreads; ++t) {
            unsigned int startRow = t * rowsPerThread;
            unsigned int endRow = (t == numThreads - 1) ? m_pixel_size.y : startRow + rowsPerThread;
            threads.emplace_back([=]() {
                for (unsigned int i = startRow; i < endRow; ++i) {
                    for (unsigned int j = 0; j < m_pixel_size.x; ++j) {
                        size_t index = j + i * m_pixel_size.x;
                        m_vArray[index].position = Vector2f(static_cast<float>(j), static_cast<float>(i));
                        Vector2f coord = mapPixelToCoords(Vector2i(j, i));
                        size_t iterations = countIterations(coord);
                        Uint8 r, g, b;
                        iterationsToRGB(iterations, r, g, b);
                        m_vArray[index].color = Color(r, g, b);
                    }
                }
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        m_State = State::DISPLAYING;
    }
}

void ComplexPlane::zoomIn() {
    ++m_zoomCount;
    float x_size = BASE_WIDTH * std::pow(BASE_ZOOM, m_zoomCount);
    float y_size = BASE_HEIGHT * m_aspectRatio * std::pow(BASE_ZOOM, m_zoomCount);
    m_plane_size = { x_size, y_size };
    m_State = State::CALCULATING;
}

void ComplexPlane::zoomOut() {
    --m_zoomCount;
    float x_size = BASE_WIDTH * std::pow(BASE_ZOOM, m_zoomCount);
    float y_size = BASE_HEIGHT * m_aspectRatio * std::pow(BASE_ZOOM, m_zoomCount);
    m_plane_size = { x_size, y_size };
    m_State = State::CALCULATING;
}

void ComplexPlane::setCenter(Vector2i mousePixel) {
    m_plane_center = mapPixelToCoords(mousePixel);
    m_State = State::CALCULATING;
}

void ComplexPlane::setMouseLocation(Vector2i mousePixel) {
    m_mouseLocation = mapPixelToCoords(mousePixel);
}

void ComplexPlane::loadText(Text& text) {
    std::stringstream ss;
    ss << "Nate and Sabreen's Mandelbrot\n";
    ss << "Cursor: (" << m_mouseLocation.x << ", " << m_mouseLocation.y << ")\n";
    ss << "Center: (" << m_plane_center.x << ", " << m_plane_center.y << ")\n";
    ss << "Left-click to Zoom in\n";
    ss << "Right-click to Zoom out\n";
    text.setString(ss.str());
}

size_t ComplexPlane::countIterations(Vector2f coord) {
    std::complex<float> z(0.0f, 0.0f);
    std::complex<float> c(coord.x, coord.y);
    size_t iterations = 0;
    while (std::abs(z) <= 2.0f && iterations < MAX_ITER) {
        z = z * z + c;
        ++iterations;
    }
    return iterations;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b) {
    if (count == MAX_ITER) {
        r = 255;
        g = 182;
        b = 193;
    }
    else if (count > MAX_ITER * 0.66f) {
        r = 231;
        g = 84;
        b = 128;
    }
    else {
        r = 255;
        g = 255;
        b = 255;
    }
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel) {
    float real_start = m_plane_center.x - m_plane_size.x / 2.0f;
    float imag_start = m_plane_center.y - m_plane_size.y / 2.0f;
    float invertedY = static_cast<float>(m_pixel_size.y) - static_cast<float>(mousePixel.y);
    float real = (static_cast<float>(mousePixel.x) / m_pixel_size.x) * m_plane_size.x + real_start;
    float imag = (invertedY / m_pixel_size.y) * m_plane_size.y + imag_start;
    return Vector2f(real, imag);
}
