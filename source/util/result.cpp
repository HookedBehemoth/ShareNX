/*
 * Copyright (c) 2019 screen-nx
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "result.hpp"

#include <iomanip>

namespace result {

std::string nintyFormat(Result rc) {
	char str[0x10];
	snprintf(str, 10, "%04d-%04d", 2000 + R_MODULE(rc), R_DESCRIPTION(rc));
	return str;
}
std::string hexFormat(Result rc) {
	std::stringstream stream;
	stream << "0x" << std::hex << rc;
	return stream.str();
}

} // namespace result