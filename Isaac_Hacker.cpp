#include <windows.h>
#undef max
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // 用于格式化输出
#include <limits>  // 用于 std::numeric_limits

using namespace std;

DWORD GetProcessID(const wstring& processName);
uintptr_t GetModuleBaseAddress(DWORD processID, const wstring& moduleName);
uintptr_t GetFinalAddress(HANDLE hProcess, uintptr_t baseAddress, const vector<unsigned int>& offsets);


// "isaac-ng.exe" + 0x007FEC30 + 0x4 + 0x20 + 0x6D8 + 0x114 + 0x1290
vector<unsigned int> offsets = { 0x4, 0x20, 0x6D8, 0x114, 0x1290 };
#pragma pack(push, 1)
struct IsaacData {
    uint32_t redHeartContainer;      // 心之容器 0x0
    uint32_t redHeart;               // 红心 0x4
    uint32_t eternalHeart;           // 永恒之心 0x8
    uint32_t soulHeart;              // 魂心 0xC

    uint8_t padding1[12];            // 12 字节填充 0x10

    uint32_t key;                    // 钥匙数量 0x1C
    bool goldKey;                    // 金钥匙 0x20
    uint8_t padding2[3];             // 3 字节填充，以对齐到 0x24
    uint32_t bomb;                   // 炸弹数量 0x24
    uint32_t gold;                   // 金币数量 0x28

    uint8_t padding3[236];           // 236 字节填充 0x2C

    float tearsDelay;                // 射击延迟 0x118
    float shotSpeed;                 // 弹速 0x11C

    uint8_t padding4[8];             // 8字节填充 0x120

    float damage;                    // 基础伤害 0x128

    uint8_t padding5[128];           // 128字节填充 0x12C

    float headHight;                 // 头高度 0x1AC
    float bodyWidth;                 // 身体宽度 0x1B0
    float bodyHight;                 // 身体高度 0x1B4
    float headWidth;                 // 头宽度 0x1B8

    uint8_t padding6[96];            // 96字节填充 0x1BC

    float speed;                     // 移速 0x21C
    float luck;                      // 幸运 0x220
};
#pragma pack(pop)
void SetCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

int main()
{
    std::setlocale(LC_ALL, "");

    const wstring processName = L"isaac-ng.exe";
    const wstring moduleName = L"isaac-ng.exe"; // 通常主模块名称与进程名相同

    // 获取进程ID
    DWORD processID = GetProcessID(processName);
    if (processID == 0) {
        wcerr << L"无法找到进程: " << processName << endl;
        wcout << L"退出中..." << endl;
        return -1;
    }

    // 打开进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processID);
    if (hProcess == NULL) {
        wcerr << L"无法打开进程句柄。" << endl;
        wcout << L"退出中..." << endl;
        return -2;
    }

    // 获取模块基地址
    uintptr_t baseAddress = GetModuleBaseAddress(processID, moduleName);
    if (baseAddress == 0) {
        wcerr << L"无法获取模块基地址。" << endl;
        CloseHandle(hProcess);
        wcout << L"退出中..." << endl;
        return -3;
    }

    // 初始地址 + 偏移基址
    uintptr_t initialAddress = baseAddress + 0x007FEC30;

    // 计算最终地址
    uintptr_t finalAddress = GetFinalAddress(hProcess, initialAddress, offsets);
    if (finalAddress == 0) {
        cerr << "无法计算最终地址。" << endl;
        CloseHandle(hProcess);
        wcout << L"退出中..." << endl;
        return -4;
    }

    // 读取整个结构体
    IsaacData isaacData;
    wcout << L"初始化..." << endl;
    // 读取游戏数据的函数
    auto ReadGameData = [&](IsaacData& data) -> bool {
        return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(finalAddress), &data, sizeof(IsaacData), nullptr);
        };

    // 写入游戏数据的函数
    auto WriteGameData = [&](const IsaacData& data) -> bool {
        return WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(finalAddress), &data, sizeof(IsaacData), nullptr);
        };

    // 初次读取数据
    if (!ReadGameData(isaacData)) {
        wcerr << L"读取数据失败。" << endl;
        CloseHandle(hProcess);
        return -5;
    }

    bool running = true;
    while (running) {
        // 将光标移动到第一行第一个位置
        SetCursorPosition(0, 0);

        cout << "==================== 以撒数据修改器 ====================" << endl;
        cout << "1. 查看当前数据" << endl;
        cout << "2. 修改心之容器" << endl;
        cout << "3. 修改红心" << endl;
        cout << "4. 修改永恒之心" << endl;
        cout << "5. 修改魂心" << endl;
        cout << "6. 修改钥匙数量" << endl;
        cout << "7. 修改炸弹数量" << endl;
        cout << "8. 修改金币数量" << endl;
        cout << "9. 修改射击延迟" << endl;
        cout << "10. 修改弹速" << endl;
        cout << "11. 修改基础伤害" << endl;
        cout << "12. 修改头高度" << endl;
        cout << "13. 修改身体宽度" << endl;
        cout << "14. 修改身体高度" << endl;
        cout << "15. 修改头宽度" << endl;
        cout << "16. 修改移速" << endl;
        cout << "17. 修改幸运" << endl;
        cout << "18. 退出程序" << endl;
        cout << "========================================================" << endl;
        cout << "请选择操作: ";

        // 使用标准输入读取用户输入
        string input;
        getline(cin, input);

        // 检查是否用户想要退出
        if (input.empty()) {
            // 如果输入为空，重新显示菜单
            continue;
        }

        try {
            int choice = stoi(input);
            switch (choice) {
            case 1:
            {
                if (ReadGameData(isaacData)) {
                    cout << "\n=== 当前游戏数据 ===" << endl;
                    cout << "心之容器: " << isaacData.redHeartContainer << endl;
                    cout << "红心: " << isaacData.redHeart << endl;
                    cout << "永恒之心: " << isaacData.eternalHeart << endl;
                    cout << "魂心: " << isaacData.soulHeart << endl;
                    cout << "钥匙: " << isaacData.key << endl;
                    cout << "炸弹: " << isaacData.bomb << endl;
                    cout << "金币: " << isaacData.gold << endl;
                    cout << "射击延迟: " << isaacData.tearsDelay << endl;
                    cout << "弹速: " << isaacData.shotSpeed << endl;
                    cout << "基础伤害: " << isaacData.damage << endl;
                    cout << "头高度: " << isaacData.headHight << endl;
                    cout << "身体宽度: " << isaacData.bodyWidth << endl;
                    cout << "身体高度: " << isaacData.bodyHight << endl;
                    cout << "头宽度: " << isaacData.headWidth << endl;
                    cout << "移速: " << isaacData.speed << endl;
                    cout << "幸运: " << isaacData.luck << endl;
                }
                else {
                    cerr << "\n读取游戏数据失败。" << endl;
                }
                break;
            }
            case 2:
            {
                cout << "\n输入新的心之容器数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
                isaacData.redHeartContainer = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "心之容器已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新心之容器失败。" << endl;
                }
                break;
            }
            case 3:
            {
                cout << "\n输入新的红心数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.redHeart = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "红心已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新红心失败。" << endl;
                }
                break;
            }
            case 4:
            {
                cout << "\n输入新的永恒之心数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.eternalHeart = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "永恒之心已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新永恒之心失败。" << endl;
                }
                break;
            }
            case 5:
            {
                cout << "\n输入新的魂心数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.soulHeart = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "魂心已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新魂心失败。" << endl;
                }
                break;
            }
            case 6:
            {
                cout << "\n输入新的钥匙数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.key = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "钥匙数量已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新钥匙数量失败。" << endl;
                }
                break;
            }
            case 7:
            {
                cout << "\n输入新的炸弹数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.bomb = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "炸弹数量已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新炸弹数量失败。" << endl;
                }
                break;
            }
            case 8:
            {
                cout << "\n输入新的金币数量: ";
                uint32_t newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.gold = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "金币数量已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新金币数量失败。" << endl;
                }
                break;
            }
            case 9:
            {
                cout << "\n输入新的射击延迟 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.tearsDelay = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "射击延迟已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新射击延迟失败。" << endl;
                }
                break;
            }
            case 10:
            {
                cout << "\n输入新的弹速 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.shotSpeed = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "弹速已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新弹速失败。" << endl;
                }
                break;
            }
            case 11:
            {
                cout << "\n输入新的基础伤害 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.damage = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "基础伤害已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新基础伤害失败。" << endl;
                }
                break;
            }
            case 12:
            {
                cout << "\n输入新的头高度 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.headHight = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "头高度已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新头高度失败。" << endl;
                }
                break;
            }
            case 13:
            {
                cout << "\n输入新的身体宽度 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.bodyWidth = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "身体宽度已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新身体宽度失败。" << endl;
                }
                break;
            }
            case 14:
            {
                cout << "\n输入新的身体高度 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.bodyHight = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "身体高度已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新身体高度失败。" << endl;
                }
                break;
            }
            case 15:
            {
                cout << "\n输入新的头宽度 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.headWidth = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "头宽度已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新头宽度失败。" << endl;
                }
                break;
            }
            case 16:
            {
                cout << "\n输入新的移速 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.speed = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "移速已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新移速失败。" << endl;
                }
                break;
            }
            case 17:
            {
                cout << "\n输入新的幸运 (float): ";
                float newValue;
                cin >> newValue;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                isaacData.luck = newValue;
                if (WriteGameData(isaacData)) {
                    cout << "幸运已更新为 " << newValue << endl;
                }
                else {
                    cerr << "更新幸运失败。" << endl;
                }
                break;
            }
            case 18:
            {
                cout << "\n退出中...";
                running = false;
                break;
            }
            default:
                cout << "\n无效的选择。" << endl;
                break;
            }
        }
        catch (const std::invalid_argument& e) {
            cout << "\n无效的输入，请输入有效的数字。" << endl;
        }
        catch (const std::out_of_range& e) {
            cout << "\n输入的数字超出范围。" << endl;
        }

        // 等待用户按键以继续
        cout << "\n按任意键继续...";
        system("pause >nul");
        system("cls"); // 清屏
    }

    // 关闭进程句柄
    CloseHandle(hProcess);
    return 0;
}


// 获取进程ID
DWORD GetProcessID(const wstring& processName) {
    DWORD processID = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        wcerr << L"创建进程快照失败。" << endl;
        return 0;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                processID = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    else {
        wcerr << L"无法获取第一个进程。" << endl;
    }

    CloseHandle(snapshot);
    return processID;
}

// 获取模块基地址
uintptr_t GetModuleBaseAddress(DWORD processID, const wstring& moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (snapshot == INVALID_HANDLE_VALUE) {
        wcerr << L"创建模块快照失败。" << endl;
        return 0;
    }

    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &moduleEntry)) {
        do {
            if (moduleName == moduleEntry.szModule) {
                baseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                break;
            }
        } while (Module32NextW(snapshot, &moduleEntry));
    }
    else {
        wcerr << L"无法获取第一个模块。" << endl;
    }

    CloseHandle(snapshot);
    return baseAddress;
}


// 计算最终地址
uintptr_t GetFinalAddress(HANDLE hProcess, uintptr_t baseAddress, const vector<unsigned int>& offsets) {
    uintptr_t addr = baseAddress;
    for (size_t i = 0; i < offsets.size(); ++i) {
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(addr), &addr, sizeof(addr), nullptr)) {
            cerr << "Failed to read memory at address: " << hex << addr << endl;
            return 0;
        }
        addr += offsets[i];
    }
    return addr;
}
