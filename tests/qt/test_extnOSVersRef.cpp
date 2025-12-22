#include "extnOSVersRef.h"
#include <iostream>

int main() {
    // Test: Find Windows OS type
    const ExtnOSVersRef* winRef = ExtnOSVersRef::findByOSType(ExtnOSType::Windows);
    if (winRef && winRef->getOSType() == ExtnOSType::Windows) {
        std::cout << "PASS: Found Windows OS type: " << winRef->getOSCompany().toStdString() << ", " << winRef->getOSTitle().toStdString() << std::endl;
    } else {
        std::cout << "FAIL: Windows OS type not found" << std::endl;
    }

    // Test: Find Unknown OS type
    const ExtnOSVersRef* unknownRef = ExtnOSVersRef::findByOSType(ExtnOSType::Unknown);
    if (unknownRef && unknownRef->getOSType() == ExtnOSType::Unknown) {
        std::cout << "PASS: Found Unknown OS type: " << unknownRef->getOSCompany().toStdString() << std::endl;
    } else {
        std::cout << "FAIL: Unknown OS type not found" << std::endl;
    }

    // Test: Find non-existent OS type
    const ExtnOSVersRef* fakeRef = ExtnOSVersRef::findByOSType(static_cast<ExtnOSType>(999));
    if (fakeRef && fakeRef->getOSType() == ExtnOSType::Unknown) {
        std::cout << "PASS: Non-existent OS type returns Unknown" << std::endl;
    } else {
        std::cout << "FAIL: Non-existent OS type did not return Unknown" << std::endl;
    }

    // Test: Find custom OS type
    const ExtnOSVersRef* linuxRef = ExtnOSVersRef::findByOSType(ExtnOSType::Linux);
    if (linuxRef && linuxRef->getOSType() == ExtnOSType::Linux) {
        std::cout << "PASS: Found Linux OS type: " << linuxRef->getOSCompany().toStdString() << std::endl;
    } else {
        std::cout << "FAIL: Linux OS type not found" << std::endl;
    }

    return 0;
}
