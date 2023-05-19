#include "Includes.h"

using jniGetCreatedJavaVMs_t = jint(*)(JavaVM** vmBuf, jsize bufLen, jsize* nVMs);

void GodModeON()
{
	const auto jvmHandle = GetModuleHandleW(L"jvm.dll");
	if (jvmHandle == nullptr) 
    { std::cout << "[!] Failed to retrieve handle for jvm.dll !\n"; return; }

	const auto jniGetCreatedJavaVMs = reinterpret_cast<jniGetCreatedJavaVMs_t>(GetProcAddress(jvmHandle, "JNI_GetCreatedJavaVMs"));

	JavaVM* javaVm = nullptr;

	jniGetCreatedJavaVMs(&javaVm, 1, nullptr);
	if (javaVm == nullptr) 
    { std::cout << "[!] Failed to retrieve created Java VMs!\n"; return; }

	JNIEnv* jniEnv = nullptr;
	javaVm->AttachCurrentThread(reinterpret_cast<void**>(&jniEnv), nullptr);
	if (jniEnv == nullptr) 
    { std::cout << "[!] Failed to attach to the Java VM.\n"; return; }

	const auto isoPlayerClass = jniEnv->FindClass("zombie/characters/IsoPlayer");
	if (isoPlayerClass == nullptr) 
    { std::cout << "[!] Failed to retrieve IsoPlayer class.\n"; return; }

	const auto hasInstanceMethodId = jniEnv->GetStaticMethodID(isoPlayerClass, "hasInstance", "()Z");
	if (hasInstanceMethodId == nullptr) 
    { std::cout << "[!] Failed to retrive IsoPlayer::hasInstance method.\n"; return; }

	const auto playerHasInstance = jniEnv->CallStaticBooleanMethod(isoPlayerClass, hasInstanceMethodId);
	if (!playerHasInstance) 
    { std::cout << "[!] IsoPlayer::hasInstance returned false.\n"; return; }

	const auto getInstanceMethodId = jniEnv->GetStaticMethodID(isoPlayerClass, "getInstance", "()Lzombie/characters/IsoPlayer;");
	if (getInstanceMethodId == nullptr) 
    { std::cout << "[!] Failed to retrieve IsoPlayer::getInstance method.\n"; return; }

	const auto playerInstance = jniEnv->CallStaticObjectMethod(isoPlayerClass, getInstanceMethodId);
    if (playerInstance == nullptr) 
    { std::cout << "[!] IsoPlayer::getInstance returned nullptr!\n"; return; }

    //*********//

    const auto setGodModMethodId = jniEnv->GetMethodID(isoPlayerClass, "setGodMod", "(Z)V");
    if (setGodModMethodId == nullptr) {std::cout << "[!] Failed to retrieve IsoGameCharacter::setGodMod method.\n";return;}

    jniEnv->CallVoidMethod(playerInstance, setGodModMethodId, 1);
}

twglSwapBuffers oSwapBuffers = NULL;
WNDPROC oWndProc;
static HWND Window = NULL;

int init = false;
bool show = false;
BOOL __stdcall hkSwapBuffers(_In_ HDC hDc)
{
    if (init == FALSE)
    {
        glewExperimental = GL_TRUE;
        if (glewInit() == GLEW_OK)
        {
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
            ImGui_ImplWin32_Init(Window);
            ImGui_ImplOpenGL3_Init();
            init = TRUE;
        }
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
        show = !show;

    if (GetAsyncKeyState(VK_END) & 1) // Unload
    {
        MH_DisableHook(MH_ALL_HOOKS);
        SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)oWndProc); // Reset WndProc
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show)
    {
        ImGui::Begin("Ici les bo boutons");
        if (ImGui::Button("God Mode ON"))
        {
            GodModeON();
        }
        ImGui::End();
   }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return oSwapBuffers(hDc);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI Initalization(__in  LPVOID lpParameter)
{
    while (GetModuleHandle("opengl32.dll") == NULL)  { Sleep(100); }
    Sleep(100);

    HMODULE hMod = GetModuleHandle("opengl32.dll");
    if (hMod)
    {
        void* ptr = GetProcAddress(hMod, "wglSwapBuffers");
        MH_Initialize();
        MH_CreateHook(ptr, hkSwapBuffers, reinterpret_cast<void**>(&oSwapBuffers));
        MH_EnableHook(ptr);
        do
            Window = GetProcessWindow();
        while (Window == NULL);
        oWndProc = (WNDPROC)SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)WndProc);
        return true;
    }
    else
        return false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, Initalization, 0, 0, 0); 
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

