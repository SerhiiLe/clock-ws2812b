// файл с переводами строк на разные языки
#ifndef CLOCK_TRANSLATIONS_H
#define CLOCK_TRANSLATIONS_H

//clock.h

// дни недели

const char PROGMEM T_sunday_en[] = "Sunday";
const char PROGMEM T_sunday_ru[] = "Воскресенье";
const char PROGMEM T_sunday_ua[] = "Неділя";

const char PROGMEM T_monday_en[] = "Monday";
const char PROGMEM T_monday_ru[] = "Понедельник";
const char PROGMEM T_monday_ua[] = "Понеділок";

const char PROGMEM T_tuesday_en[] = "Tuesday";
const char PROGMEM T_tuesday_ru[] = "Вторник";
const char PROGMEM T_tuesday_ua[] = "Вівторок";

const char PROGMEM T_wednesday_en[] = "Wednesday";
const char PROGMEM T_wednesday_ru[] = "Среда";
const char PROGMEM T_wednesday_ua[] = "Середа";

const char PROGMEM T_thursday_en[] = "Thursday";
const char PROGMEM T_thursday_ru[] = "Четверг";
const char PROGMEM T_thursday_ua[] = "Четвер";

const char PROGMEM T_friday_en[] = "Friday";
const char PROGMEM T_friday_ru[] = "Пятница";
const char PROGMEM T_friday_ua[] = "П'ятниця";

const char PROGMEM T_saturday_en[] = "Saturday";
const char PROGMEM T_saturday_ru[] = "Суббота";
const char PROGMEM T_saturday_ua[] = "Субота";

const char* PROGMEM txt_week_l[LANGUAGES][7] = {
    {T_sunday_en, T_monday_en, T_tuesday_en, T_wednesday_en, T_thursday_en, T_friday_en, T_saturday_en},
    {T_sunday_ru, T_monday_ru, T_tuesday_ru, T_wednesday_ru, T_thursday_ru, T_friday_ru, T_saturday_ru},
    {T_sunday_ua, T_monday_ua, T_tuesday_ua, T_wednesday_ua, T_thursday_ua, T_friday_ua, T_saturday_ua}
};

// в разных языках меняется порядок выводимых данных! Требуется поддержка в коде.
const char PROGMEM F_day_of_week_en[] = "%s, %s %u, %u";
const char PROGMEM F_day_of_week_ru[] = "%s %u %s %u года";
const char PROGMEM F_day_of_week_ua[] = "%s %u %s %u року";
const char* PROGMEM F_date_l[LANGUAGES] = {F_day_of_week_en, F_day_of_week_ru, F_day_of_week_ua};

// месяцы

const char PROGMEM T_january_en[] = "January";
const char PROGMEM T_january_ru[] = "января";
const char PROGMEM T_january_ua[] = "січня";

const char PROGMEM T_february_en[] = "February";
const char PROGMEM T_february_ru[] = "февраля";
const char PROGMEM T_february_ua[] = "лютого";

const char PROGMEM T_march_en[] = "March";
const char PROGMEM T_march_ru[] = "марта";
const char PROGMEM T_march_ua[] = "березня";

const char PROGMEM T_april_en[] = "April";
const char PROGMEM T_april_ru[] = "апреля";
const char PROGMEM T_april_ua[] = "квітня";

const char PROGMEM T_may_en[] = "May";
const char PROGMEM T_may_ru[] = "мая";
const char PROGMEM T_may_ua[] = "травня";

const char PROGMEM T_june_en[] = "June";
const char PROGMEM T_june_ru[] = "июня";
const char PROGMEM T_june_ua[] = "червня";

const char PROGMEM T_july_en[] = "July";
const char PROGMEM T_july_ru[] = "июля";
const char PROGMEM T_july_ua[] = "липня";

const char PROGMEM T_august_en[] = "August";
const char PROGMEM T_august_ru[] = "августа";
const char PROGMEM T_august_ua[] = "серпня";

const char PROGMEM T_september_en[] = "September";
const char PROGMEM T_september_ru[] = "сентября";
const char PROGMEM T_september_ua[] = "вересня";

const char PROGMEM T_october_en[] = "October";
const char PROGMEM T_october_ru[] = "октября";
const char PROGMEM T_october_ua[] = "жовтня";

const char PROGMEM T_november_en[] = "November";
const char PROGMEM T_november_ru[] = "ноября";
const char PROGMEM T_november_ua[] = "листопада";

const char PROGMEM T_december_en[] = "December";
const char PROGMEM T_december_ru[] = "декабря";
const char PROGMEM T_december_ua[] = "грудня";

const char* PROGMEM txt_month[LANGUAGES][12] = {
    {T_january_en, T_february_en, T_march_en, T_april_en, T_may_en, T_june_en,
     T_july_en, T_august_en, T_september_en, T_october_en, T_november_en, T_december_en},
    {T_january_ru, T_february_ru, T_march_ru, T_april_ru, T_may_ru, T_june_ru,
     T_july_ru, T_august_ru, T_september_ru, T_october_ru, T_november_ru, T_december_ru},
    {T_january_ua, T_february_ua, T_march_ua, T_april_ua, T_may_ua, T_june_ua,
     T_july_ua, T_august_ua, T_september_ua, T_october_ua, T_november_ua, T_december_ua}
};

// короткие названия дней недели, существует два варианта и два символа. Последнее считается стандартом.
// но красивее три символа. Чтобы не сбивался вывод, отсутствующий символ заменен на широкий пробел.

const char PROGMEM T_sun2_en[] = "Sun";
const char PROGMEM T_sun2_ru[] = "\x7fВс";
const char PROGMEM T_sun2_ua[] = "\x7fНд";
const char PROGMEM T_mon2_en[] = "Mon";
const char PROGMEM T_mon2_ru[] = "\x7fПн";
const char PROGMEM T_tue2_en[] = "Tue";
const char PROGMEM T_tue2_ru[] = "\x7fВт";
const char PROGMEM T_wed2_en[] = "Wed";
const char PROGMEM T_wed2_ru[] = "\x7fСр";
const char PROGMEM T_thu2_en[] = "Thu";
const char PROGMEM T_thu2_ru[] = "\x7fЧт";
const char PROGMEM T_fri2_en[] = "Fri";
const char PROGMEM T_fri2_ru[] = "\x7fПт";
const char PROGMEM T_sat2_en[] = "Sat";
const char PROGMEM T_sat2_ru[] = "\x7fСб";

// Нестандартный, но более красивый вариант сокращения до трёх букв

const char PROGMEM T_sun3_en[] = "\x7fSu";
const char PROGMEM T_sun3_ru[] = "Вск";
const char PROGMEM T_sun3_ua[] = "Ндл";
const char PROGMEM T_mon3_en[] = "\x7fMo";
const char PROGMEM T_mon3_ru[] = "Пнд";
const char PROGMEM T_tue3_en[] = "\x7fTu";
const char PROGMEM T_tue3_ru[] = "Втр";
const char PROGMEM T_wed3_en[] = "\x7fWe";
const char PROGMEM T_wed3_ru[] = "Срд";
const char PROGMEM T_thu3_en[] = "\x7fTh";
const char PROGMEM T_thu3_ru[] = "Чтв";
const char PROGMEM T_fri3_en[] = "\x7f\x46r";
const char PROGMEM T_fri3_ru[] = "Птн";
const char PROGMEM T_sat3_en[] = "\x7fSa";
const char PROGMEM T_sat3_ru[] = "Сбт";

const char* PROGMEM txt_week_short[2][LANGUAGES][7] = {
    {
        {T_sun2_en, T_mon2_en, T_tue2_en, T_wed2_en, T_thu2_en, T_fri2_en, T_sat2_en},
        {T_sun2_ru, T_mon2_ru, T_tue2_ru, T_wed2_ru, T_thu2_ru, T_fri2_ru, T_sat2_ru},
        {T_sun2_ua, T_mon2_ru, T_tue2_ru, T_wed2_ru, T_thu2_ru, T_fri2_ru, T_sat2_ru}
    }, {
        {T_sun3_en, T_mon3_en, T_tue3_en, T_wed3_en, T_thu3_en, T_fri3_en, T_sat3_en},
        {T_sun3_ru, T_mon3_ru, T_tue3_ru, T_wed3_ru, T_thu3_ru, T_fri3_ru, T_sat3_ru},
        {T_sun3_ua, T_mon3_ru, T_tue3_ru, T_wed3_ru, T_thu3_ru, T_fri3_ru, T_sat3_ru}
    }
};

// короткие названия месяцев, не по стандарту, потому что все должны быть 3 буквы. Увы, но нет приемлемых общепринятых сокращений для украинского языка.

const char PROGMEM T_jan_en[] = "Jan";
const char PROGMEM T_jan_ru[] = "янв";
const char PROGMEM T_jan_ua[] = "січ";

const char PROGMEM T_feb_en[] = "Feb";
const char PROGMEM T_feb_ru[] = "фев";
const char PROGMEM T_feb_ua[] = "лют";

const char PROGMEM T_mar_en[] = "Mar";
const char PROGMEM T_mar_ru[] = "мар";
const char PROGMEM T_mar_ua[] = "бер";

const char PROGMEM T_apr_en[] = "Apr";
const char PROGMEM T_apr_ru[] = "апр";
const char PROGMEM T_apr_ua[] = "кві";

const char PROGMEM T_may3_en[] = "May";
const char PROGMEM T_may3_ru[] = "май";
const char PROGMEM T_may3_ua[] = "тра";

const char PROGMEM T_jun_en[] = "Jun";
const char PROGMEM T_jun_ru[] = "июн";
const char PROGMEM T_jun_ua[] = "чер";

const char PROGMEM T_jul_en[] = "Jul";
const char PROGMEM T_jul_ru[] = "июл";
const char PROGMEM T_jul_ua[] = "лип";

const char PROGMEM T_aug_en[] = "Aug";
const char PROGMEM T_aug_ru[] = "авг";
const char PROGMEM T_aug_ua[] = "сер";

const char PROGMEM T_sep_en[] = "Sep";
const char PROGMEM T_sep_ru[] = "сен";
const char PROGMEM T_sep_ua[] = "вер";

const char PROGMEM T_oct_en[] = "Oct";
const char PROGMEM T_oct_ru[] = "окт";
const char PROGMEM T_oct_ua[] = "жов";

const char PROGMEM T_nov_en[] = "Nov";
const char PROGMEM T_nov_ru[] = "ноя";
const char PROGMEM T_nov_ua[] = "лис";

const char PROGMEM T_dec_en[] = "Dec";
const char PROGMEM T_dec_ru[] = "дек";
const char PROGMEM T_dec_ua[] = "гру";

const char* PROGMEM txt_month_short[LANGUAGES][12] = {
    {T_jan_en, T_feb_en, T_mar_en, T_apr_en, T_may3_en, T_jun_en,
     T_jul_en, T_aug_en, T_sep_en, T_oct_en, T_nov_en, T_dec_en},
    {T_jan_ru, T_feb_ru, T_mar_ru, T_apr_ru, T_may3_ru, T_jun_ru,
     T_jul_ru, T_aug_ru, T_sep_ru, T_oct_ru, T_nov_ru, T_dec_ru},
    {T_jan_ua, T_feb_ua, T_mar_ua, T_apr_ua, T_may3_ua, T_jun_ua,
     T_jul_ua, T_aug_ua, T_sep_ua, T_oct_ua, T_nov_ua, T_dec_ua}
};

#endif