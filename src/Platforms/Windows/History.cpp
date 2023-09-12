#include "History.h"
#include "Utils.h"

void getEXEPathAndDirectory(History* history) {
	DWORD sizeofFilePath{ GetModuleFileNameA(0, history->exeFilePath, sizeof(history->exeFilePath)) };

	if (sizeofFilePath > 2) {
		for (uint32_t i{ sizeofFilePath - 1 }; i > 1; i--) {
			if (history->exeFilePath[i] == '\\') {
				history->exeDirectory = &history->exeFilePath[i] + 1;
				break;
			}
		}
	}
}

void buildFileInEXEDirectory(History* history, char* filename, int dstCount, char* dst) {
	ASSERT(history->exeDirectory);
	ASSERT(history->exeFilePath)

		if (!history->exeDirectory) {
			getEXEPathAndDirectory(history);
		}
	catString(history->exeFilePath, history->exeDirectory - history->exeFilePath,
		filename, static_cast<size_t>(stringLength(filename)),
		dst, dstCount
	);
}