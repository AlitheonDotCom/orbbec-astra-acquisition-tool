#include <cstdio>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <astra/astra.hpp>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <SFML/Graphics.hpp>
#include <key_handler.h>
#include "LitDepthVisualizer.hpp"


class DepthFrameListener : public astra::FrameListener
{
public:
    DepthFrameListener(const astra::CoordinateMapper& coordinateMapper)
    : coordinateMapper_(coordinateMapper)
    {
        prev_ = ClockType::now();
        font_.loadFromFile("Inconsolata.otf");
    }
    
    void init_texture(int width, int height)
    {
        if (!displayBuffer_ ||
            width != displayWidth_ ||
            height != displayHeight_)
        {
            displayWidth_ = width;
            displayHeight_ = height;
            
            // texture is RGBA
            const int byteLength = displayWidth_ * displayHeight_ * 4;
            
            displayBuffer_ = BufferPtr(new uint8_t[byteLength]);
            std::fill(&displayBuffer_[0], &displayBuffer_[0] + byteLength, 0);
            
            texture_.create(displayWidth_, displayHeight_);
            sprite_.setTexture(texture_);
            sprite_.setPosition(0, 0);
        }
    }
    
    void check_fps()
    {
        const float frameWeight = .2f;
        
        const ClockType::time_point now = ClockType::now();
        const float elapsedMillis = std::chrono::duration_cast<DurationType>(now - prev_).count();
        
        elapsedMillis_ = elapsedMillis * frameWeight + elapsedMillis_ * (1.f - frameWeight);
        prev_ = now;
        
        const float fps = 1000.f / elapsedMillis;
        
        const auto precision = std::cout.precision();
        
        std::cout << std::fixed
        << std::setprecision(1)
        << fps << " fps ("
        << std::setprecision(1)
        << elapsedMillis_ << " ms)"
        << std::setprecision(precision)
        << std::endl;
    }
    
    void savePCDFile(const astra::PointFrame& pointFrame)
    {
        
        pcl::PointCloud<pcl::PointXYZ> cloud;
        std::string fileName;
        
        // Fill in the cloud data
        cloud.width    = pointFrame.width();
        cloud.height   = pointFrame.height();
        cloud.is_dense = false;
        cloud.points.resize (cloud.width * cloud.height);
        
        const astra::Vector3f* frameData=pointFrame.data();
        
        for (size_t i = 0; i < cloud.points.size (); ++i)
        {
            cloud.points[i].x = frameData[i].x;
            cloud.points[i].y = frameData[i].y;
            cloud.points[i].z = frameData[i].z;
        }
        std::cout<<"Enter the name of the PCD file to be saved: ";
        std::cin>>fileName;
        
        pcl::io::savePCDFileASCII (fileName+".pcd", cloud);
        std::cerr << "Saved " << cloud.points.size () << " data points to test_pcd.pcd." << std::endl;
        
        for (size_t i = 0; i < cloud.points.size (); ++i)
            std::cerr << "    " << cloud.points[i].x << " " << cloud.points[i].y << " " << cloud.points[i].z << std::endl;
        
        savePCD_=false;
        
    }
    
    void get_center_pixel_depth(astra::Frame& frame)
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();
        const int width = depthFrame.width();
        const int height = depthFrame.height();
        
        const size_t middleIndex = ((width * (height / 2.f)) + (width / 2.f));
        
        const short* frameData = depthFrame.data();
        centerPixelDepth_ = frameData[middleIndex];
    }
    
    void on_frame_ready(astra::StreamReader& reader,
                        astra::Frame& frame) override
    {
        const astra::PointFrame pointFrame = frame.get<astra::PointFrame>();
        
        const int width = pointFrame.width();
        const int height = pointFrame.height();
        
        init_texture(width, height);
        
        check_fps();
        
        if (isPaused_) { return; }
        
        if (savePCD_){savePCDFile(pointFrame);}
        
        copy_depth_data(frame);
        get_center_pixel_depth(frame);
        
        visualizer_.update(pointFrame);
        
        const astra::RgbPixel* vizBuffer = visualizer_.get_output();
        for (int i = 0; i < width * height; i++)
        {
            const int rgbaOffset = i * 4;
            displayBuffer_[rgbaOffset] = vizBuffer[i].r;
            displayBuffer_[rgbaOffset + 1] = vizBuffer[i].b;
            displayBuffer_[rgbaOffset + 2] = vizBuffer[i].g;
            displayBuffer_[rgbaOffset + 3] = 255;
        }
        
        texture_.update(displayBuffer_.get());
    }
    
    void copy_depth_data(astra::Frame& frame)
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();
        
        if (depthFrame.is_valid())
        {
            const int width = depthFrame.width();
            const int height = depthFrame.height();
            if (!depthData_ || width != depthWidth_ || height != depthHeight_)
            {
                depthWidth_ = width;
                depthHeight_ = height;
                
                // texture is RGBA
                const int byteLength = depthWidth_ * depthHeight_ * sizeof(uint16_t);
                
                depthData_ = DepthPtr(new int16_t[byteLength]);
            }
            
            depthFrame.copy_to(&depthData_[0]);
        }
    }
    
    void update_mouse_position(sf::RenderWindow& window)
    {
        const sf::Vector2i position = sf::Mouse::getPosition(window);
        const sf::Vector2u windowSize = window.getSize();
        
        mouseNormX_ = position.x / float(windowSize.x);
        mouseNormY_ = position.y / float(windowSize.y);
    }
    
    void draw_text(sf::RenderWindow& window,
                   sf::Text& text,
                   sf::Color color,
                   const int x,
                   const int y) const
    {
        text.setColor(sf::Color::Black);
        text.setPosition(x + 5, y + 5);
        window.draw(text);
        
        text.setColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }
    
    void draw_mouse_overlay(sf::RenderWindow& window,
                            const float depthWScale,
                            const float depthHScale) const
    {
        if (!isMouseOverlayEnabled_ || !depthData_) { return; }
        
        const int mouseX = depthWidth_ * mouseNormX_;
        const int mouseY = depthHeight_ * mouseNormY_;
        
        if (mouseX >= depthWidth_ ||
            mouseY >= depthHeight_ ||
            mouseX < 0 ||
            mouseY < 0) { return; }
        
        const size_t index = (depthWidth_ * mouseY + mouseX);
        const short z = depthData_[index];
        
        float worldX, worldY, worldZ;
        coordinateMapper_.convert_depth_to_world(float(mouseX),
                                                 float(mouseY),
                                                 float(z),
                                                 &worldX,
                                                 &worldY,
                                                 &worldZ);
        std::stringstream sstr;
        int characterSize;
        const float displayX=10.f;
        float displayY;
        float margin;
        
        if (displayHelpText_)
        {
            sstr<<"_____________HELP MENU_____________"<<std::endl;
            sstr<<"Hit 'Esc' to close the application."<<std::endl
            <<"Press 'Control+C' to close the application."<<std::endl
            <<"Press 'F/f' to toggle between full screen."<<std::endl
            <<"Press 'P/p' to pause/un-pause the depth stream."<<std::endl
            <<"Press 'A/a' to save a PCD file for the latest depth frame."<<std::endl
            <<"Press 'R/r' to enable/disable registration."<<std::endl
            <<"Press 'M/m' to mirror the depth stream."<<std::endl
            <<"Hit 'Space' to toggle between overlaid text."<<std::endl;
            
            //set the font size to 25 to allow for a cleaner output
            characterSize=25;
            
            //set the display coordinates to display the help menu
            margin=250.f;
            displayY = window.getView().getSize().y - (margin + characterSize);
            
        }
        else
        {
            sstr << std::fixed
            << std::setprecision(0)
            << "(" << mouseX << ", " << mouseY << ") "
            << "X: " << worldX << " Y: " << worldY << " Z: " << worldZ<<" Center pixel depth: "<<centerPixelDepth_;
            sstr<<std::endl<<"Press 'H/h to toggle between the help menu.";
            
            //set the font size to 40
            characterSize=40;
            
            //set the display coordinates to display the help menu
            margin=80.f;
            displayY = window.getView().getSize().y - (margin + characterSize);
        }
        
        
        //const int characterSize = 40;
        sf::Text text(sstr.str(), font_);
        text.setCharacterSize(characterSize);
        text.setStyle(sf::Text::Bold);
        
        //const float displayX = 10.f;
        //const float margin = 10.f;
        //const float displayY = window.getView().getSize().y - (margin + characterSize);
        
        draw_text(window, text, sf::Color::White, displayX, displayY);
    }
    
    void draw_to(sf::RenderWindow& window)
    {
        if (displayBuffer_ != nullptr)
        {
            const float depthWScale = window.getView().getSize().x / displayWidth_;
            const float depthHScale = window.getView().getSize().y / displayHeight_;
            
            sprite_.setScale(depthWScale, depthHScale);
            window.draw(sprite_);
            
            draw_mouse_overlay(window, depthWScale, depthHScale);
        }
    }
    
    void toggle_paused()
    {
        isPaused_ = !isPaused_;
    }
    
    bool is_paused() const
    {
        return isPaused_;
    }
    
    void toggle_overlay()
    {
        isMouseOverlayEnabled_ = !isMouseOverlayEnabled_;
    }
    
    bool overlay_enabled() const
    {
        return isMouseOverlayEnabled_;
    }
    
    void acquire()
    {
        savePCD_=true;
    }
    
    void displayHelp()
    {
        displayHelpText_=!displayHelpText_;
    }
    
    /*
    void print_depth_frame(const astra::PointFrame& pointFrame) const
    {
        const int frameIndex = pointFrame.frame_index();
        const astra::Vector3f middleValue = get_middle_value(pointFrame);
        
        std::printf("Depth frameIndex: %d x: %f y: %f z:%f \n \n", frameIndex, middleValue.x, middleValue.y, middleValue.z);
    }
    
    astra::Vector3f get_middle_value(const astra::PointFrame& pointFrame) const
    {
        const int width = pointFrame.width();
        const int height = pointFrame.height();
        
        std::cout<<"point frame width"<<width<<"\n";
        std::cout<<"point frame height"<<height<<"\n";
        
        const size_t middleIndex = ((width * (height / 2.f)) + (width / 2.f));
        
        const astra::Vector3f* frameData = pointFrame.data();
        const astra::Vector3f middleValue = frameData[middleIndex];
        
        
        return middleValue;
    }*/
    
private:
    samples::common::LitDepthVisualizer visualizer_;
    
    using DurationType = std::chrono::milliseconds;
    using ClockType = std::chrono::high_resolution_clock;
    
    ClockType::time_point prev_;
    float elapsedMillis_{.0f};
    
    sf::Texture texture_;
    sf::Sprite sprite_;
    sf::Font font_;
    
    const astra::CoordinateMapper& coordinateMapper_;
    
    int displayWidth_{0};
    int displayHeight_{0};
    
    using BufferPtr = std::unique_ptr<uint8_t[]>;
    BufferPtr displayBuffer_{nullptr};
    
    int depthWidth_{0};
    int depthHeight_{0};
    
    using DepthPtr = std::unique_ptr<int16_t[]>;
    DepthPtr depthData_{nullptr};
    
    short centerPixelDepth_{0};
    
    float mouseNormX_{0};
    float mouseNormY_{0};
    bool isPaused_{false};
    bool savePCD_{false};
    bool isMouseOverlayEnabled_{true};
    bool displayHelpText_{false};
};


int main(int argc, char** argv)
{
    astra::initialize();
    set_key_handler();
    
    sf::RenderWindow window(sf::VideoMode(1280, 960), "Depth Viewer");
    
#ifdef _WIN32
    auto fullscreenStyle = sf::Style::None;
#else
    auto fullscreenStyle = sf::Style::Fullscreen;
#endif
    
    const sf::VideoMode fullScreenMode = sf::VideoMode::getFullscreenModes()[0];
    const sf::VideoMode windowedMode(1280, 1024);
    bool isFullScreen = false;

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();
    
    reader.stream<astra::PointStream>().start();
    auto depthStream = reader.stream<astra::DepthStream>();
    depthStream.start();
    
    auto coordinateMapper = depthStream.coordinateMapper();
    
    DepthFrameListener listener(coordinateMapper);
    
    reader.add_listener(listener);
    
    while (window.isOpen())
    {
        astra_temp_update();
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                {
                    if (event.key.code == sf::Keyboard::C && event.key.control)
                    {
                        window.close();
                    }
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::F:
                            if (isFullScreen)
                            {
                                window.create(windowedMode, "Depth Viewer", sf::Style::Default);
                            }
                            else
                            {
                                window.create(fullScreenMode, "Depth Viewer", fullscreenStyle);
                            }
                            isFullScreen = !isFullScreen;
                            break;
                        case sf::Keyboard::R:
                            depthStream.enable_registration(!depthStream.registration_enabled());
                            break;
                        case sf::Keyboard::M:
                            depthStream.enable_mirroring(!depthStream.mirroring_enabled());
                            break;
                        case sf::Keyboard::P:
                            listener.toggle_paused();
                            break;
                        case sf::Keyboard::A:
                            listener.acquire();
                            break;
                        case sf::Keyboard::H:
                            listener.displayHelp();
                            break;
                        case sf::Keyboard::Space:
                            listener.toggle_overlay();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case sf::Event::MouseMoved:
                {
                    listener.update_mouse_position(window);
                    break;
                }
                default:
                    break;
            }
        }
        
        // clear the window with black color
        window.clear(sf::Color::Black);
        
        listener.draw_to(window);
        window.display();
        
        if (!shouldContinue)
        {
            window.close();
        }
    }

    //reader.remove_listener(listener);
    

    astra::terminate();
    
    return 0;
}

