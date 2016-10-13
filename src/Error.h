#ifndef _ERROR_H_
#define _ERROR_H_

namespace sinl {

class Error {
public:

	enum errcode {
		GENERIC = 1,
		INVVER,
	};

    std::string message(enum errcode value) const {
        switch (value) {
            case errcode::GENERIC:
                return "Generic error";
			case errcode::INVVER:
                return "Invalid library version";
            default:
                return "Unknown";
        }
    }

};

} // namespace sinl

#endif // _ERROR_H_
