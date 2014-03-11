#include "Game.h"

namespace Library
{
    const UINT Game::DefaultScreenWidth = 1024;
    const UINT Game::DefaultScreenHeight = 768;

    Game::Game(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        : mInstance(instance), mWindowClass(windowClass), mWindowTitle(windowTitle), mShowCommand(showCommand),
          mWindowHandle(), mWindow(),
          mScreenWidth(DefaultScreenWidth), mScreenHeight(DefaultScreenHeight),
          mGameClock(), mGameTime()
    {
    }

    Game::~Game()
    {
    }

    HINSTANCE Game::Instance() const
    {
        return mInstance;
    }

    HWND Game::WindowHandle() const
    {
        return mWindowHandle;
    }

    const WNDCLASSEX& Game::Window() const
    {
        return mWindow;
    }

    const std::wstring& Game::WindowClass() const
    {
        return mWindowClass;
    }

    const std::wstring& Game::WindowTitle() const
    {
        return mWindowTitle;
    }

    int Game::ScreenWidth() const
    {
        return mScreenWidth;
    }

    int Game::ScreenHeight() const
    {
        return mScreenHeight;
    }
        
    void Game::Run()
    {
        InitializeWindow();
        Initialize();

        MSG message;
        ZeroMemory(&message, sizeof(message));
        
        mGameClock.Reset();		

        while(message.message != WM_QUIT)
        {
            if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                mGameClock.UpdateGameTime(mGameTime);
                Update(mGameTime);
                Draw(mGameTime);
            }
        }

		Shutdown();
    }

    void Game::Exit()
    {
        PostQuitMessage(0);
    }

    void Game::Initialize()
    {
    }

    void Game::Update(const GameTime& gameTime)
    {
    }

    void Game::Draw(const GameTime& gameTime)
    {
    }

    void Game::InitializeWindow()
    {
        ZeroMemory(&mWindow, sizeof(mWindow));
        mWindow.cbSize = sizeof(WNDCLASSEX);
        mWindow.style = CS_CLASSDC;
        mWindow.lpfnWndProc = WndProc;
        mWindow.hInstance = mInstance;
        mWindow.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        mWindow.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        mWindow.hCursor = LoadCursor(nullptr, IDC_ARROW);
        mWindow.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
        mWindow.lpszClassName = mWindowClass.c_str();		

        RECT windowRectangle = { 0, 0, mScreenWidth, mScreenHeight };
        AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

        RegisterClassEx(&mWindow);
        POINT center = CenterWindow(mScreenWidth, mScreenHeight);
        mWindowHandle = CreateWindow(mWindowClass.c_str(), mWindowTitle.c_str(), WS_OVERLAPPEDWINDOW, center.x, center.y, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, nullptr, nullptr, mInstance, nullptr);

        ShowWindow(mWindowHandle, mShowCommand);
        UpdateWindow(mWindowHandle);
    }

	void Game::Shutdown()
	{
		UnregisterClass(mWindowClass.c_str(), mWindow.hInstance);
	}

    LRESULT WINAPI Game::WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch(message)
        {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }

        return DefWindowProc(windowHandle, message, wParam, lParam);
    }

    POINT Game::CenterWindow(int windowWidth, int windowHeight)
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        POINT center;
        center.x = (screenWidth - windowWidth) / 2;
        center.y = (screenHeight - windowHeight) / 2;

        return center;
    }	
}