#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
//---------------------------------------------------
// Глобальные переменные
//---------------------------------------------------
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;



ID3D11Buffer* triangleVertBuffer;   //Буфер который содержит информацию о vertex
ID3D11VertexShader* VS;             //vertex шейдер
ID3D11PixelShader* PS;              //pixel шейдер
ID3D10Blob* VS_Buffer;              //vertex буфер
ID3D10Blob* PS_Buffer;              //vertex буфер
ID3D11InputLayout* vertLayout;      //input (vertex) layout


struct Vertex	//Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z)
		: pos(x, y, z) {}

	XMFLOAT3 pos;
};



D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION",       //    This is just a string to associate with the element. This string will be used to map the elements in the vertex structure to the elements in the vertex shader.
	0,                  //    This is basically just a number after the semantic name to use as an index. For example, if we have 2 texture elements in the vertex structure, instead of creating 2 different texture semantic names, we can just use 2 different index's. If a semantic name in the vertex shader code has no index after it, it defaults to index 0. For example in our shader code, our semantic name is "POSITION", which is actually the same as "POSITION0".
	DXGI_FORMAT_R32G32B32_FLOAT, //      This is just the format of our component in our vertex structure. It needs to be a member of the DXGI_FORMAT enumerated type. In this lesson, we have a 3d vector describing the position, so we can use the DXGI_FORMAT: DXGI_FORMAT_R32G32B32_FLOAT. If you need other formats, you can find them on msdn. Later we will be using other ones.
	0,                  //    Direct3D allows us to use 16 different element slots (0-15) which you can put vertex data through. If we have our vertex structure has a position and color, we could put both the elements through the same input slot, or we can put the position data through the first slot, and the color data through the second slot. We only need to use one, but you can experiment if you would like.
	0,                  //    This is the byte offset of the element you are describing. In a single input slot, if we have position and color, position could be 0 since it starts at the beginning of the vertex structer, and color would need to be the size of our vertex position, which is 12 bytes (remember our format for our vertex position is DXGI_FORMAT_R32G32B32_FLOAT, which is 96 bits, 32 for each component in the position. there are 8 bits in one byte, so 96/8 == 12).
	D3D11_INPUT_PER_VERTEX_DATA, // Right now we can just use D3D10_INPUT_PER_VERTEX_DATA. The other options are used for instancing, which is an advanced technique we will get to later.
	0 },                //    This is also used only for instancing, so we will specify 0 for now
};
UINT numElements = ARRAYSIZE(layout);   // после того как мы определили input layout, мы создаем глобальную переменную для хранения
										// размера массива input layout. мы это делаем для того чтобы позже нам не надо было бы помнить продолжать обновлять функцию которая создает input layout





float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;




//---------------------------------------------------
// Проототипы Функций
//---------------------------------------------------
bool InitializeDirect3d11App(HINSTANCE hInstance); // Для инициализации direct3d
void ReleaseObjects(); // Для освобождения объектов для не допускания утечки памяти
bool InitScene(); // Настройка сцены
void UpdateScene(); // Изменить нашу сцену на покадровую основу
void DrawScene(); // Нарисовать сцену на экран и обновлять каждый кадр


LPCTSTR WndClassName = "firstwindow1";
HWND hwnd = NULL;

const int Width  = 800;
const int Height = 600;

bool InitializeWindow(HINSTANCE hInstance,
		 int ShowWnd,
		 int width, int height,
		 bool windowed);

int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
			 UINT msg,
			 WPARAM wParam,
			 LPARAM lParam);



int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
		   HINSTANCE hPrevInstance,
		   LPSTR lpCmdLine,
		   int nShowCmd)
{
    if(!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, "Window Initialization - Failed",
        			"Error", MB_OK);
		return 0;
	}


    if(!InitializeDirect3d11App(hInstance)) //Инициализация Direct3D
    {
            MessageBox(0, "Direct3D Initialization - Failed",
                    "Error", MB_OK);
            return 0;
    }

    if(!InitScene()) //Инициализация нашей сцены
    {
            MessageBox(0, "Scene Initialization - Failed",
                    "Error", MB_OK);
            return 0;
    }


	messageloop();
	ReleaseObjects();
	return 0;
}


bool InitializeWindow(HINSTANCE hInstance,
			 int ShowWnd,
			 int width, int height,
			 bool windowed)
{
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WndClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Error registering class",
			"Error", MB_OK | MB_ICONERROR);
		return 1;
	}


	hwnd = CreateWindowEx(NULL, WndClassName, "Window Title",
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                        width, height,
                        NULL, NULL, hInstance, NULL);


    if (!hwnd)
	{
		MessageBox(NULL, "Error creating window",
			"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

        ShowWindow(hwnd, ShowWnd);
		UpdateWindow(hwnd);

	return true;
}

int messageloop(){

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while(true)
    {
       
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
			);

		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {

            if (msg.message == WM_QUIT)
            break;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        else{
            // run game code
            UpdateScene();
            DrawScene();

            }
        }
    return static_cast<int>(msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd,
			 UINT msg,
			 WPARAM wParam,
			 LPARAM lParam)
{


    switch( msg )
{

	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE ){
			if(MessageBox(0, "Are you sure you want to exit?",
                	"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			DestroyWindow(hwnd);
		}
	return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
	return 0;
}

return DefWindowProc(hwnd,
			 msg,
			 wParam,
			 lParam);
}

//---------------------------------------------------
// Функция которая инициализирует direct3d, она берет один параметр,
// который содержит ссылку на наше приложение
//---------------------------------------------------
bool InitializeDirect3d11App(HINSTANCE hInstance)
{
        HRESULT hr; // Объект который служит для проверок на ошибки
        // Описать наш буфер
        DXGI_MODE_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC)); //отчищаем структуру т.к мы еще не установили
                                                         //все параметры и параметр имеет значение в ней

        bufferDesc.Width = Width; //ширина используемого разрешения
        bufferDesc.Height = Height; // высота
        bufferDesc.RefreshRate.Numerator = 60; //частота обновления
        bufferDesc.RefreshRate.Denominator = 1;//       60/1=60 герц
        bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //формат дисплея, 32битный unsigned integer, на каждый по 8 бит на красный, зеленый,синий и альфа

        bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //describing the manner in which the rasterizer will render onto a surface. As
                                                                            //we use double buffering, this will not usually be seen, so we can set it to DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, meaning the order in which
                                                                            //the render takes place on a surface does not matter.

        bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;                 //This is another enumerated type. The DXGI_MODE_SCALING, explaining how an image is stretched to fit a monitors resolution. We can use
                                                                            //one of three: DXGI_MODE_SCALING_UNSPECIFIED, which means it is not specified, DXGI_MODE_SCALING_CENTERED, meaning the image is centered
                                                                            //on the screen, and no scaling and stretching is done at all, and DXGI_MODE_SCALING_STRETCHED, which will stretch the image to the monitors
                                                                            //resolution.
        // Описать SwapChain
//---------------------------------------------------
// Когда мы заполнили описание заднего буфера, мы можем перейти к
// заполнению SwapChain
//---------------------------------------------------
        DXGI_SWAP_CHAIN_DESC swapChainDesc;

        ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC)); //отчистка

        swapChainDesc.BufferDesc = bufferDesc; //структура описывающая задний буфер, мы укажем bufferDesc который мы только что заполнили
        swapChainDesc.SampleDesc.Count = 1; //структура описывающая мулитисемплинг чтобы добавить гладкости линиям и границам
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //enumerated type describing the access the cpu has to the surface of the back buffer. We specify DXGI_USAGE_RENDER_TARGET_OUTPUT
                                                                     //since we will render to it
        swapChainDesc.BufferCount = 1; //Количество задних буферов, 1 - двойной буфер
        swapChainDesc.OutputWindow = hwnd; //указывает на наше окно
        swapChainDesc.Windowed = TRUE; //true для оконного режима
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //enum тип описывающий что драйвер дисплея должен делать с передним буфером после замены его на задний буфер
                                                             //we set DXGI_SWAP_EFFECT_DISCARD to let the display driver decide what the most efficient thing to do with it is.
        //Создать SwapChain
        hr = D3D11CreateDeviceAndSwapChain(
                NULL, //Указатель на используемый видео адаптер. NULL - выбрать по умолчанию
                D3D_DRIVER_TYPE_HARDWARE, //говорит как direct3d будет выполнено. сейчас указывает на выполнение с помощью GPU
                NULL,  //Указывает на DLL который будет использован для выполнения растровой рестеризации
                NULL, //This is one or more D3D11_CREATE_DEVICE_FLAG types or'ed together.
                NULL, //Указывает на массив D3D_FEATURE_LEVEL enumerated types который говорит какую версию directx использовать. NULL для последней доступной версии
                NULL, // количество элементов в массиве pFeatureLevels.
                D3D11_SDK_VERSION, //Используемая версия SDK
                &swapChainDesc, //Указатель на нашу созданную структуру DXGI_SWAP_CHAIN_DESC
                &SwapChain, //Указатель на IDXGISwapChain interface чтобы получить созданную SwapChain
                &d3d11Device, //Указатель на direct3d device
                NULL, //указатель ны D3D_FEATURE_LEVEL which will hold the highest feature level available. (Feature levels are used for backwards compatibility)
                &d3d11DevCon); //указатель на ID3D11DeviceContext (device context). Remember the device context will be used for the rendering methods of the device, to
                               //support multi-threading and boost performance.

        //Создать BackBuffer (заднийбуфер)
//---------------------------------------------------
// создаем задний буфер который будет использоваться для создания зоны видимости
//---------------------------------------------------
        ID3D11Texture2D* BackBuffer;
        hr = SwapChain->GetBuffer(
                            0,  // так как мы установили swapChainDesc.SwapEffect to DXGI_SWAP_EFFECT_DISCARD у нас есть доступ только к первому буферу, так что ставим 0
                            __uuidof( ID3D11Texture2D ), //This is a reference ID to the type of interface to change the back buffer. We use a 2d texture (ID3D11Texture2D).
                            (void**)&BackBuffer ); // Указатель на задний буфер который был создан ранее, который является поверхностью на которой будет происходить рендер

//---------------------------------------------------
// Now we create our render target view, which we will send to the output merger stage of the pipeline. We can create our render target view by calling
// CreateRenderTargetView of our device interface:
//---------------------------------------------------
        //Создать Render Target
        hr = d3d11Device->CreateRenderTargetView(
                                 BackBuffer, //это наш задний буфер
                                 NULL, //  A pointer to a D3D11_RENDER_TARGET_VIEW_DESC structure. We set NULL to create a view that accesses all of the subresources in mipmap level 0.
                                 &renderTargetView); //This is a pointer to our ID3D11RenderTargetView interface, renderTargetView.

        BackBuffer->Release(); // теперь когда нам больше не нужен задний буфер, мы можем его освободить

        //Set our Render Target
//---------------------------------------------------
// And the last thing we do while initializing, is bind the render target view to the output merger stage of the pipeline. This function will also bind our
// depth/stencil buffer as well, but we have not created one yet, so we set that parameter to NULL.
//---------------------------------------------------
        d3d11DevCon->OMSetRenderTargets( 1, //This is the number of render targets to bind. We only have one
                                        &renderTargetView, //This is an array of render target views to bind.
                                        NULL ); //This is a pointer to the depth stencil buffer. We do not have one yet, so we set this one to NULL.

		return true;
}

												
//---------------------------------------------------
// это всё для основной инициализации directx 11
//---------------------------------------------------


//---------------------------------------------------
// Следующим шагом мы отчистим наши функции.
//---------------------------------------------------
void ReleaseObjects() // освободит все созданные СОМ объекты, если этого не сделать будет утечка памяти
{
        //Release the COM Objects we created
        SwapChain->Release();
        d3d11Device->Release();
        d3d11DevCon->Release();

		triangleVertBuffer->Release();
		VS->Release();
		PS->Release();
		VS_Buffer->Release();
		PS_Buffer->Release();
		vertLayout->Release();



}


//---------------------------------------------------
// Функция используется для инициализации нашей сцены
//---------------------------------------------------
bool InitScene()
{
	HRESULT hr = S_OK;
	//Compile Shaders from shader file
	hr = D3DX11CompileFromFile("VS.hlsl",
		0,
		0,
		"VS",
		"vs_5_0",
		0,
		0,
		0,
		&VS_Buffer,
		0,
		0);

	hr = D3DX11CompileFromFile("PS.hlsl", // строка содержащая имя файла в котором находятся шейдеры
		0,                // указатель на macros. NULL
		0,                // указатель на include интерфейс, если бы шейлер использовал #include в файле, мы бы не ставили NULL, но наш шейдер не имеет include, так что мы ставим NULL
		"PS",             // имя шейдерной функции в file name
		"ps_5_0",         // версия используемого шейдера. dx11 поддерживает версию 5.0
		0,                // Compile flags, we will set this to NULL.
		0,                // Effect flags. We will also set this to NULL.
		0,                // This has to do with multi-threading. We set NULL so the function will not return until it has completed.
		&PS_Buffer,       // This is the returned shader. It is not the actual shader, but more like a buffer containing the shader and information about the shader. We will then use this buffer to create the actual shader.
		0,                // This returns the list of errors and warnings that happened while compiling the shader. These errors and warnings are the same that you can see in the bottom of the debugger.
		0);               // This is the returned HRESULT. We did it so "hr = " this function, but you can also put "&hr" for this parameter to do the same thing.

						  //Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), // указатель на начало шейдерного буфера
		VS_Buffer->GetBufferSize(),       // размер буфера
		NULL,                             // A pointer to a class linkage interface. We will set this to NULL.
		&VS);                             // This is our returned vertex shader.

	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(),
		PS_Buffer->GetBufferSize(),
		NULL,
		&PS);                              // This is our returned pixel shader.

										   //Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS,  // This is our Vertex Shader.
		0,      // This is only used if our shader uses and interface. Set to NULL.
		0);     // This is the number of class-instances in the array from ppClassInstances. We set to 0 because there isn't one.

	d3d11DevCon->PSSetShader(PS,  // This is our Pixe Shader.
		0,      //
		0);     //

				//Create the vertex buffer
	Vertex v[] =
	{
		Vertex(0.0f, 0.5f, 0.5f),
		Vertex(0.5f, -0.5f, 0.5f),
		Vertex(-0.5f, -0.5f, 0.5f),
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;           // A D3D11_USAGE type describing how our buffer will be read from and written to.
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;      // This is the size in bytes of our buffer.
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // We specify D3D11_BIND_VERTEX_BUFFER since this is a vertex buffer.
	vertexBufferDesc.CPUAccessFlags = 0;                    // This says how our buffer will be used by the CPU. We can set this to NULL
	vertexBufferDesc.MiscFlags = 0;                         // Extra flags we will not use, set this to NULL too
															//vertexBufferDesc.StructureByteStride = 0;     //           Not used here, set this to NULL.

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;                           // This is the data we want to put into the buffer.

	hr = d3d11Device->CreateBuffer(
		&vertexBufferDesc,      // Pointer to our buffer description
		&vertexBufferData,      // Pointer to a subresource data structure containing the data we want to put here. We can set this to NULL if we want to add data later.
		&triangleVertBuffer);   // The returned ID3D11Buffer.

								//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//Now that we have a vertex buffer, we need to bind it to the IA. We can do this by calling the function ID3D11Devicecontext::IASetVertexBuffers function:
	d3d11DevCon->IASetVertexBuffers(0,                 // This is the input slot we can bind it to. We set 0 here.
		1,                     // This is the number of buffers we are binding. We are only binding 1.
		&triangleVertBuffer,   // This is a pointer to our actual vertex buffer.
		&stride,               // This is the size of each vertex.
		&offset);             // This is an ofset in bytes from the beginning of the buffer of where to start.

							  //Create the Input Layout
	hr = d3d11Device->CreateInputLayout(layout,        // This is the array of D3D11_INPUT_ELEMENT_DESC elements that contain our vertex layout.
		numElements,       // This is the number of elements in our vertex layout.
		VS_Buffer->GetBufferPointer(), // This is a pointer to the start of our Vertex Shader.
		VS_Buffer->GetBufferSize(), // This is the size of our vertex shader.
		&vertLayout);     //    This is the returned pointer to our input (vertex) layout.

						  //Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertLayout); //     Our created input layout.

											   //Set Primitive Topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //Here we create triangle. each triangle shares its vertices with the adjacent triangles. All triangles will be connected.

																				//Create the Viewport
																				/*Now all thats left to do is create and set our viewport. The viewport will tell the RS stage of the pipeline
																				what to draw. We can create a viewport using the D3D11_VIEWPORT structure.
																				The viewport creates a square in pixels, which the rasterizer uses to find where
																				o display our geometry on the client area of our window. You will also use the
																				viewport when we introduce the depth buffer. We can set the minimum and maximum
																				depth values, usually between 0 and 1. Then the OM will decide which pixel "fragments"
																				to display based on their depth values. We want the viewport to cover our entire window
																				client area, so we set the top left of the box to 0,0, and the bottom right of our box
																				to Width,Height, which are in pixels.*/

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;

	//Set the Viewport
	/*After we have created our viewport, we need to bind it to the RS stage of the
	pipeline using the function ID3D11DeviceContext::RSSetViewports().
	The first parameter is the number of viewports to bind, and the second
	is a pointer to an array of viewports. This is where you could
	have multiple "windows", like one for player one and one for player 2.*/

	d3d11DevCon->RSSetViewports(1, &viewport);

	return true;
}


//---------------------------------------------------
// Функция используется для обновления сцены
//---------------------------------------------------
void UpdateScene()
{
        //Update the colors of our scene
        red += colormodr * 0.00005f;
        green += colormodg * 0.00002f;
        blue += colormodb * 0.00001f;
        if(red >= 1.0f || red <= 0.0f)
        colormodr *= -1;
        if(green >= 1.0f || green <= 0.0f)
        colormodg *= -1;
        if(blue >= 1.0f || blue <= 0.0f)
        colormodb *= -1;
}


//---------------------------------------------------
// Now we have our DrawScene() function. This is where we will simply render our scene. We should avoid doing any updating in this scene, and keep
// this function only for drawing our scene. This is where we will change the color of our background. Remember, if you have come from an earlier
// version of DirectX, like directx 10, all the rendering methods have been moved to the device context interface, so instead of calling d3dDevice like we
// would have in directx 10, we will call our d3dDeviceContext object to get the ClearRenderTargetView method, since this is a rendering method. We
// will call d3dDevice for other things that have to do with the GPU, other than rendering. Finally, we present the scene by calling the Present method
// of our swapchain interface. What this does is swap the front buffer with the back buffer. In the drawscene function, we will render to the backbuffer,
// then the back buffer is presented when call the Present method
//---------------------------------------------------
void DrawScene()
{
        //Clear our backbuffer to the updated color
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };        
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	d3d11DevCon->Draw(3, 0);
	
	//Present the backbuffer to the screen
        SwapChain->Present(0, 0);
}
