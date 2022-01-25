#include "preferences.h"

const WifiConfiguration WIFI_CONFIGURATION = { "SSID_HERE", "PASSWORD_HERE" };

// +10  -> Sydney, Melbourne, Yakutsk, Port Moresby
// +9   -> Seoul, Tokyo
// +8   -> Perth, Beijing, Manila, Singapore, Kuala Lumpur
// +7   -> Jakarta, Bangkok, Phnom Pehn, Hanoi
// +6   -> Astana, Almaty, Bishkek, Dhaka
// +5   -> Karachi, Lahore, Faisalabad, Tashkent
// +4   -> Baku, Tbilisi, Yerevan, Dubai, Moscow
// +3   -> Nairobi, Baghdad, Khartoum, Mogadishu, Riyadh
// +2   -> Athens, Sofia, Cairo, Riga, Istanbul, Helsinki, Jerusalem
// +1   -> Amsterdam, Belgrade, Berlin, Budapest, Vienna, Prague, Brussels
// +0   -> London, Dublin, Abidjan, Casablanca, Accra, Lisbon
// -4   -> Santiago de Chile, La Paz, San Juan de Puerto Rico
// -5   -> ET, New York, Washington D.C., Atlanta, Miami, Qeubec, Toronto
// -6   -> CT, Belize City, Chicago, Dallas, Guadalajara, Guatemala City
// -7   -> MT, Denver, Phoenix, Salt Lake City, Calgary
// -8   -> PT, Seattle, Portland, San Francisco, Los Angeles, Vancouver
// -9   -> Anchorage, Juneau, Mangareva
// -10  -> Papeete, Honolulu

const int UTC_OFFSET = -7;

// Refer to https://en.cppreference.com/w/c/chrono/strftime for c date string formats
// Some popular formats:
//      October 09, 2012     -> "%B %d, %Y"
//      09 Oct 2012          -> "%d %b, %Y"
//      10/09/2012           -> "%D" or "%m/%d/%Y"
//      09/10/12             -> "%d/%m/%y"

const char DATE_FORMAT[] = "%d %b";
const char TIME_FORMAT[] = "%H:%M";