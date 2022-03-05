/*
 Copyright (C) 2021 Olli Salonen (gpl-3.0-or-later)
               2022 Fredrik Öhrström (gpl-3.0-or-later)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include"meters_common_implementation.h"

struct MeterMinomess : public virtual MeterCommonImplementation
{
    MeterMinomess(MeterInfo &mi, DriverInfo &di);

private:

    double total_water_consumption_m3_ {};
    string meter_date_;
    double target_water_consumption_m3_ {};
    string target_date_;
    string status_;
};

static bool ok = registerDriver([](DriverInfo&di)
{
    di.setName("minomess");
    di.setMeterType(MeterType::WaterMeter);
    di.setExpectedELLSecurityMode(ELLSecurityMode::AES_CTR);
    di.addLinkMode(LinkMode::C1);
    di.addDetection(MANUFACTURER_ZRI, 0x07,  0x00);
    di.setConstructor([](MeterInfo& mi, DriverInfo& di){ return shared_ptr<Meter>(new MeterMinomess(mi, di)); });
});

MeterMinomess::MeterMinomess(MeterInfo &mi, DriverInfo &di) : MeterCommonImplementation(mi, di)
{
    addFieldWithExtractor(
        "total",
        Quantity::Volume,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::Volume,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total water consumption recorded by this meter.",
        SET_FUNC(total_water_consumption_m3_, Unit::M3),
        GET_FUNC(total_water_consumption_m3_, Unit::M3));

    /* If the meter is recently commissioned, the target water consumption value is bogus.
       The bits store 0xffffffff. Should we deal with this? Now a very large value is printed in the json. */

    addStringFieldWithExtractor(
        "meter_date",
        Quantity::Text,
        NoDifVifKey,
        MeasurementType::Instantaneous,
        ValueInformation::Date,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON,
        "Date when measurement was recorded.",
        SET_STRING_FUNC(meter_date_),
        GET_STRING_FUNC(meter_date_));

    addFieldWithExtractor(
        "target",
        Quantity::Volume,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::Volume,
        StorageNr(8),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total water consumption recorded at the beginning of this month.",
        SET_FUNC(target_water_consumption_m3_, Unit::M3),
        GET_FUNC(target_water_consumption_m3_, Unit::M3));

    addStringFieldWithExtractor(
        "target_date",
        Quantity::Text,
        NoDifVifKey,
        MeasurementType::Instantaneous,
        ValueInformation::Date,
        StorageNr(8),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON,
        "Date when target water consumption was recorded.",
        SET_STRING_FUNC(target_date_),
        GET_STRING_FUNC(target_date_));

    /*
    According to data sheet, there are two status/info bytes, byte A and byte B.
    Unfortunately we do not now is byte A is the first or second byte. Oh well.
    Now we guess that A is the hi byte. I.e. 0x8000 is byte A bit 7.
    In the telegram the byte order is: lo byte first followed by the hi byte.
    So the defacto telegram bytes would be 0x0080 for byte A bit 7.

    Byte A:
    bit 7 removal active in the past
    bit 6 tamper active in the past
    bit 5 leak active in the past
    bit 4 temporary error (in connection with smart functions)
    bit 3 permanent error (meter value might be lost)
    bit 2 battery EOL (measured)
    bit 1 abnormal error
    bit 0 unused

    Byte B:
    bit 7 burst
    bit 6 removal
    bit 5 leak
    bit 4 backflow in the past
    bit 3 backflow
    bit 2 meter blocked in the past
    bit 1 meter undersized
    bit 0 meter oversized
    */

    addStringFieldWithExtractorAndLookup(
        "status",
        Quantity::Text,
        DifVifKey("02FD17"),
        MeasurementType::Unknown,
        ValueInformation::Any,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "Status and error flags.",
        SET_STRING_FUNC(status_),
        GET_STRING_FUNC(status_),
         {
            {
                {
                    "ERROR_FLAGS",
                    Translate::Type::BitToString,
                    0xffff,
                    "OK",
                    {
                        { 0x8000, "WAS_REMOVED" },
                        { 0x4000, "WAS_TAMPERED" },
                        { 0x2000, "WAS_LEAKING" },
                        { 0x1000, "TEMPORARY_ERROR" },
                        { 0x0800, "PERMANENT_ERROR" },
                        { 0x0400, "BATTERY_EOL" },
                        { 0x0200, "ABNORMAL_ERROR" },
                        // 0x0100 not used
                        { 0x0080, "BURSTING" },
                        { 0x0040, "REMOVED" },
                        { 0x0020, "LEAKING" },
                        { 0x0010, "WAS_BACKFLOWING" },
                        { 0x0008, "BACKFLOWING" },
                        { 0x0004, "WAS_BLOCKED" },
                        { 0x0002, "UNDERSIZED" },
                        { 0x0001, "OVERSIZED" }
                    }
                },
            },
         });

}

    // 00: 66 length (102 bytes)
    // 01: 44 dll-c (from meter SND_NR)
    // 02: 496a dll-mfct (ZRI)
    // 04: 10640355 dll-id (55036410)
    // 08: 14 dll-version
    // 09: 37 dll-type (Radio converter (meter side))
    // 0a: 72 tpl-ci-field (EN 13757-3 Application Layer (long tplh))
    // 0b: 51345015 tpl-id (15503451)
    // 0f: 496a tpl-mfct (ZRI)
    // 11: 00 tpl-version
    // 12: 07 tpl-type (Water meter)
    // 13: 76 tpl-acc-field
    // 14: 00 tpl-sts-field (OK)
    // 15: 5005 tpl-cfg 0550 (AES_CBC_IV nb=5 cntn=0 ra=0 hc=0 )
    // 17: 2f2f decrypt check bytes

    // 19: 0C dif (8 digit BCD Instantaneous value)
    // 1a: 13 vif (Volume l)
    // 1b: * 55140000 total consumption (1.455000 m3)
    // 1f: 02 dif (16 Bit Integer/Binary Instantaneous value)
    // 20: 6C vif (Date type G)
    // 21: * A92B meter date (2021-11-09)
    // 23: 82 dif (16 Bit Integer/Binary Instantaneous value)
    // 24: 04 dife (subunit=0 tariff=0 storagenr=8)
    // 25: 6C vif (Date type G)
    // 26: * A12B target consumption reading date (2021-11-01)
    // 28: 8C dif (8 digit BCD Instantaneous value)
    // 29: 04 dife (subunit=0 tariff=0 storagenr=8)
    // 2a: 13 vif (Volume l)
    // 2b: * 71000000 target consumption (0.071000 m3)
    //
    // 2f: 8D dif (variable length Instantaneous value)
    // 30: 04 dife (subunit=0 tariff=0 storagenr=8)
    // 31: 93 vif (Volume l)
    // 32: 13 vife (Reverse compact profile without register)
    // 33: 2C varlen=44
    //  This register has 24-bit integers for the consumption of the past months n-2 until n-15.
    //  If the meter is commissioned less than 15 months ago, you will see FFFFFF as the value.
    //          n-2    n-3    n-4    n-5    n-6    n-7    n-8    n-9    n-10   n-11   n-12   n-13   n-14   n-15
    // 34: FBFE 000000 FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF FFFFFF
    //
    // 60: 02 dif (16 Bit Integer/Binary Instantaneous value)
    // 61: FD vif (Second extension FD of VIF-codes)
    // 62: 17 vife (Error flags (binary))
    // 63: * 0000 info codes (OK)


// Test: Mino minomess 15503451 NOKEY
// telegram=|6644496A1064035514377251345015496A0007EE0050052F2F#0C1359000000026CBE2B82046CA12B8C0413FFFFFFFF8D0493132CFBFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF02FD1700002F2F|
// {"media":"water","meter":"minomess","name":"Mino","id":"15503451","total_m3":0.059,"meter_date":"2021-11-30","target_m3":244444.442,"target_date":"2021-11-01","status":"OK","timestamp":"1111-11-11T11:11:11Z"}
// |Mino;15503451;0.059000;244444.442000;OK;1111-11-11 11:11.11
