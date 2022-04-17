#include <Aurora/Core/String.hpp>
#include <Aurora/Core/AUID.hpp>
#include <Aurora/Logger/std_sink.hpp>
#include <Aurora/Resource/ResourceName.hpp>

using namespace Aurora;

// *

int main()
{
	Logger::AddSink<std_sink>();

	AU_LOG_INFO("Random: ", (String)Aurora::AUID::AUID());
	if(Aurora::AUID::IsValid<String>("0de49ab9-2c9d-b155-70e6-0c9067010000"))
	{
		Aurora::AUID uuid = Aurora::AUID::FromString<String>("0de49ab9-2c9d-b155-70e6-0c9067010000").value();
		Aurora::AUID uuid2 = Aurora::AUID::FromString<String>((String)uuid).value();

		if(uuid == uuid2)
		{
			AU_LOG_INFO("matches !");
		}

		AU_LOG_INFO("Rand: ", (String)uuid);
		AU_LOG_INFO("Rand: ", (String)uuid2);
	}

	{
		ResourceName rn("Test");
		AU_LOG_INFO((String)rn.ID, " - ", rn.Name);
	}

	{
		ResourceName rn("{0de49ab9-2c9d-b155-70e6-0c9067010000}Test");
		AU_LOG_INFO((String)rn.ID, " - ", rn.Name);
	}

	return 0;
}