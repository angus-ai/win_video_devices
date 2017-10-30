#define NOMINMAX

#include <windows.h>
#include <dshow.h>
#include <unordered_map>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#pragma comment(lib, "strmiids")


// convert a BSTR to a std::string. 
std::string& BstrToStdString(const BSTR bstr, std::string& dst, int cp = CP_UTF8)
{
    if (!bstr)
    {
        // define NULL functionality. I just clear the target.
        dst.clear();
        return dst;
    }

    // request content length in single-chars through a terminating
    //  nullchar in the BSTR. note: BSTR's support imbedded nullchars,
    //  so this will only convert through the first nullchar.
    int res = WideCharToMultiByte(cp, 0, bstr, -1, NULL, 0, NULL, NULL);
    if (res > 0)
    {
        dst.resize(res-1);
        WideCharToMultiByte(cp, 0, bstr, res-1, &dst[0], res, NULL, NULL);
    }
    else
    {    // no content. clear target
        dst.clear();
    }
    return dst;
}

// conversion with temp.
std::string BstrToStdString(BSTR bstr, int cp = CP_UTF8)
{
    std::string str;
    BstrToStdString(bstr, str, cp);
    return str;
}

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}


std::vector<std::pair<int, std::string>> DisplayDeviceInformation(IEnumMoniker *pEnum)
{
    IMoniker *pMoniker = NULL;
    int i = 0;
    std::vector<std::pair<int, std::string>> dict;
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr))
        {
            dict.push_back(make_pair(i, BstrToStdString(var.bstrVal)));
            VariantClear(&var);
        }

        hr = pPropBag->Write(L"FriendlyName", &var);

        pPropBag->Release();
        pMoniker->Release();

        i++;
    }

    return dict;
}

std::vector<std::pair<int, std::string>> get_devices()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    std::vector<std::pair<int, std::string>> tt;
    if (SUCCEEDED(hr))
    {
        IEnumMoniker *pEnum;

        hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
        if (SUCCEEDED(hr))
        {
            tt = DisplayDeviceInformation(pEnum);
            pEnum->Release();
        }
        CoUninitialize();
    }

    return tt;
}

void main()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        IEnumMoniker *pEnum;

        hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
        if (SUCCEEDED(hr))
        {
            std::vector<std::pair<int, std::string>> devices = DisplayDeviceInformation(pEnum);
            for(std::vector<std::pair<int, std::string>>::iterator it = devices.begin(); it != devices.end(); ++it) {
                std::cout << it->first << ": " << it->second << std::endl;
            }
            pEnum->Release();
        }
        CoUninitialize();
    }
}


PYBIND11_MODULE(win_devices, m) {
    m.doc() = "Windows plugin to get video devices list"; // optional module docstring

    m.def("get_devices", &get_devices, "A function which returns devices");
}