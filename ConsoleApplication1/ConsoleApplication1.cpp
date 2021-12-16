// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "Timer.h"
#include <iostream>
using namespace winrt::Windows::Media::Audio;
using namespace winrt::Windows::System::Threading;

std::pair<AudioGraph, AudioFrameOutputNode> Initialize(bool add_frame_output_node);

int main()
{
	constexpr bool ADD_FRAME_OUTPUT_NODE = true;
	auto [audioGraph, frameOutputNode] = Initialize(ADD_FRAME_OUTPUT_NODE);
	std::atomic_int32_t samples{ 0 };
	
	//Timer timer1;
	audioGraph.QuantumStarted(
		[&](auto, auto)
		{
			winrt::Windows::Foundation::IMemoryBufferReference reference = frameOutputNode.
				GetFrame().
				LockBuffer(winrt::Windows::Media::AudioBufferAccessMode::Read).
				CreateReference();
			uint8_t* value{};
			uint32_t value_size{};
			winrt::check_hresult(reference.as<IMemoryBufferByteAccess>()->GetBuffer(&value, &value_size));

			auto num_samples = value_size / sizeof(float);
			samples.fetch_add((int)num_samples);
			//timer1.time();
		}
	);


	audioGraph.Start();

	//Timer timer2;
	ThreadPoolTimer::CreatePeriodicTimer([&](auto) 
		{
			samples.fetch_add(-4410); //ThreadPoolTimer doesn't work properly for 10ms so do 100ms
			std::cout << samples.operator int() << std::endl;
			//timer2.time();
		},
		std::chrono::milliseconds(100)); //ThreadPoolTimer doesn't work properly for 10ms so do 100ms

	std::getchar();
}

std::wstring FullPath(wchar_t const* file_name);
struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
{
	virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
};



std::pair<AudioGraph, AudioFrameOutputNode> Initialize(bool add_frame_output_node)
{
	AudioGraphSettings settings{ winrt::Windows::Media::Render::AudioRenderCategory::Media };
	winrt::Windows::Media::MediaProperties::AudioEncodingProperties encoding{};
	encoding.BitsPerSample(32);
	encoding.ChannelCount(1);
	encoding.SampleRate(44100);
	encoding.Bitrate(encoding.BitsPerSample() * encoding.ChannelCount() * encoding.SampleRate());
	encoding.Subtype(L"Float");

	settings.EncodingProperties(encoding);
	{
		using namespace winrt::Windows::Devices::Enumeration;
		auto renderDevices = DeviceInformation::FindAllAsync(DeviceClass::AudioRender).get();
		_ASSERT(renderDevices.Size() > 0);
		auto renderer = renderDevices.GetAt(std::rand()%renderDevices.Size());
		std::wcout << renderer.Name().c_str() << std::endl;
		settings.PrimaryRenderDevice(renderer);
	}
	CreateAudioGraphResult result = AudioGraph::CreateAsync(settings).get();

	WINRT_ASSERT(result.Status() == AudioGraphCreationStatus::Success);
	auto audioGraph = result.Graph();

	auto frameOutputNode = audioGraph.CreateFrameOutputNode(audioGraph.EncodingProperties());

	using winrt::Windows::Storage::StorageFile;
	StorageFile audioFile = StorageFile::GetFileFromPathAsync(FullPath(L"Music.mp3")).get();
	auto fileInputNodeResult = audioGraph.CreateFileInputNodeAsync(audioFile).get();
	WINRT_ASSERT(fileInputNodeResult.Status() == AudioFileNodeCreationStatus::Success);

	auto fileInputNode = fileInputNodeResult.FileInputNode();
	fileInputNode.AddOutgoingConnection(frameOutputNode);


	if (add_frame_output_node)
	{
		auto deviceOutputNodeResult = audioGraph.CreateDeviceOutputNodeAsync().get();
		WINRT_ASSERT(deviceOutputNodeResult.Status() == AudioDeviceNodeCreationStatus::Success);
		auto deviceOutputNode = deviceOutputNodeResult.DeviceOutputNode();
		fileInputNode.AddOutgoingConnection(deviceOutputNode);

	}
	return { audioGraph, frameOutputNode };
}

std::wstring FullPath(wchar_t const* file_name)
{
	std::wstring s;
	s.resize(200);
	auto size = GetModuleFileName(nullptr, s.data(), (DWORD)s.size());
	auto it = std::find(s.rbegin(), s.rend(), u'\\');
	auto resize = std::distance(it, s.rend());
	s.resize(resize);
	s.append(file_name);
	return s;
}
