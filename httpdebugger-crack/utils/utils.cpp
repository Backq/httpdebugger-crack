#include <windows.h>
#include <regex>
#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>

#include "utils.hh"
#include "regedit.hh"
#include <format>

std::string utils::get_regex(std::regex rx, std::string text) {
	std::smatch matched_rx;
	std::regex_search(text, matched_rx, rx);

	if (!matched_rx.str(0).empty())
		return matched_rx.str(0);
}

void utils::get_app_version() {
    RegEdit re{ RegEdit::Key::KEY_CURRENT_USER, "SOFTWARE\\MadeForNet\\HTTPDebuggerPro" };
    std::string version{ re.getString("AppVer").c_str() };

    const std::regex ver_rx("(\\d+.*)");
    utils::parsed_version = utils::get_regex(ver_rx, version);
    utils::parsed_version.erase(std::remove(utils::parsed_version.begin(), utils::parsed_version.end(), '.'), utils::parsed_version.end());
}

void utils::get_serial_number() {
    // sub_4161E0((int)v38, (int)"SN%d", v19 ^ ((~Version >> 1) + 0x2E0) ^ 0x590D4);
    // 
    DWORD volume_info = GetVolumeInformationA("C:\\", NULL, 0, &volume_info, NULL, NULL, NULL, 0) ? volume_info : GetVersion();
    uint32_t value = std::atoi(utils::parsed_version.c_str());

    uint32_t serialnumber = value ^ ((~volume_info >> 1) + 0x2E0) ^ 0x590D4;

    utils::serial_number = std::to_string(serialnumber);
}

void utils::create_key()
{
    /* [ KEY LENGTH CHECK + 7C ]   
    * 
    if ( !sub_421B20(v8, Src) || v8[0] != 0x7C )
    return 0;
    */

    /* [ KEY ]
    * 
    v2 = ~*(_BYTE *)(a2 + 3);
    v7 = 0;
    *(_BYTE *)(a2 + 1) = v2;
    *(_BYTE *)(a2 + 2) = *(_BYTE *)a2 ^ *(_BYTE *)(a2 + 7);
    v4 = *(_BYTE *)(a2 + 6);
    *(_BYTE *)(a2 + 4) = *(_BYTE *)(a2 + 3) ^ ~v4;
    *(_BYTE *)(a2 + 5) = v4 ^ 7;
    */

    std::string key = "";
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 255);

    while (key.length() != 16) {
        int v1 = dist(random);
        int v2 = dist(random);
        int v3 = dist(random);

        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << v1 << std::setw(2)
            << (v2 ^ 0x7C) << std::setw(2)
            << (0xff ^ v1) << "7C" << std::setw(2)
            << v2 << std::setw(2)
            << (v3 % 255) << std::setw(2)
            << ((v3 % 255) ^ 7) << std::setw(2)
            << (v1 ^ (0xff ^ (v3 % 255)));
        key = ss.str();
    }

    utils::generated_key = key.c_str();
}

void utils::write_key() {
    utils::get_app_version();
    utils::get_serial_number();
    utils::create_key();

    printf("[ < ] parsed_version: %s\n", utils::parsed_version);
    printf("[ < ] serial_number: SN%s\n", utils::serial_number);
    printf("[ < ] generated_key: %s\n", utils::generated_key.c_str());

    RegEdit re{ RegEdit::Key::KEY_CURRENT_USER, "SOFTWARE\\MadeForNet\\HTTPDebuggerPro" };
    re.setString("SN" + utils::serial_number, utils::generated_key.c_str());

    printf("\t[ + ] done.\n");
}