/*
 Copyright (C) 2019-2022 Fredrik Öhrström (gpl-3.0-or-later)

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

struct MeterAmiplus : public virtual MeterCommonImplementation
{
    MeterAmiplus(MeterInfo &mi, DriverInfo &di);

private:

    double total_energy_consumption_kwh_ {};
    double current_power_consumption_kw_ {};
    double total_energy_production_kwh_ {};
    double current_power_production_kw_ {};
    double phase_1_v_ {};
    double phase_2_v_ {};
    double phase_3_v_ {};
    string device_date_time_;
    double total_energy_consumption_tariff_1_kwh_ {};
    double total_energy_consumption_tariff_2_kwh_ {};
    double total_energy_consumption_tariff_3_kwh_ {};
    double total_energy_production_tariff_1_kwh_ {};
    double total_energy_production_tariff_2_kwh_ {};
    double total_energy_production_tariff_3_kwh_ {};
};

static bool ok = registerDriver([](DriverInfo&di)
{
    di.setName("amiplus");
    di.setMeterType(MeterType::ElectricityMeter);
    di.setExpectedTPLSecurityMode(TPLSecurityMode::AES_CBC_IV);
    di.addLinkMode(LinkMode::T1);
    di.addDetection(MANUFACTURER_APA,  0x02,  0x02);
    di.addDetection(MANUFACTURER_DEV,  0x37,  0x02);
    di.addDetection(MANUFACTURER_DEV,  0x02,  0x00);
    di.setConstructor([](MeterInfo& mi, DriverInfo& di){ return shared_ptr<Meter>(new MeterAmiplus(mi, di)); });
});

MeterAmiplus::MeterAmiplus(MeterInfo &mi, DriverInfo &di) : MeterCommonImplementation(mi, di)
{
    addFieldWithExtractor(
        "total_energy_consumption",
        Quantity::Energy,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::EnergyWh,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total energy consumption recorded by this meter.",
        SET_FUNC(total_energy_consumption_kwh_, Unit::KWH),
        GET_FUNC(total_energy_consumption_kwh_, Unit::KWH));

    addFieldWithExtractor(
        "current_power_consumption",
        Quantity::Power,
        NoDifVifKey,
        VifScaling::Auto,
        MeasurementType::Instantaneous,
        ValueInformation::PowerW,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "Current power consumption.",
        SET_FUNC(current_power_consumption_kw_, Unit::KW),
        GET_FUNC(current_power_consumption_kw_, Unit::KW));

    addFieldWithExtractor(
        "total_energy_production",
        Quantity::Energy,
        DifVifKey("0E833C"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "The total energy production recorded by this meter.",
        SET_FUNC(total_energy_production_kwh_, Unit::KWH),
        GET_FUNC(total_energy_production_kwh_, Unit::KWH));

    addFieldWithExtractor(
        "current_power_production",
        Quantity::Power,
        DifVifKey("0BAB3C"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::Any,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "Current power production.",
        SET_FUNC(current_power_production_kw_, Unit::KW),
        GET_FUNC(current_power_production_kw_, Unit::KW));

    addFieldWithExtractor(
        "voltage_at_phase_1",
        Quantity::Voltage,
        DifVifKey("0AFDC9FC01"),
        VifScaling::None,
        MeasurementType::Unknown,
        ValueInformation::Any,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "Voltage at phase L1.",
        SET_FUNC(phase_1_v_, Unit::Volt),
        GET_FUNC(phase_1_v_, Unit::Volt));

    addFieldWithExtractor(
        "voltage_at_phase_2",
        Quantity::Voltage,
        DifVifKey("0AFDC9FC02"),
        VifScaling::None,
        MeasurementType::Unknown,
        ValueInformation::Any,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "Voltage at phase L2.",
        SET_FUNC(phase_2_v_, Unit::Volt),
        GET_FUNC(phase_2_v_, Unit::Volt));

    addFieldWithExtractor(
        "voltage_at_phase_3",
        Quantity::Voltage,
        DifVifKey("0AFDC9FC03"),
        VifScaling::None,
        MeasurementType::Unknown,
        ValueInformation::Any,
        AnyStorageNr,
        AnyTariffNr,
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
        "Voltage at phase L3.",
        SET_FUNC(phase_3_v_, Unit::Volt),
        GET_FUNC(phase_3_v_, Unit::Volt));

    addStringFieldWithExtractor(
        "device_date_time",
        Quantity::Text,
        NoDifVifKey,
        MeasurementType::Instantaneous,
        ValueInformation::DateTime,
        StorageNr(0),
        TariffNr(0),
        IndexNr(1),
        PrintProperty::JSON,
        "Device date time.",
        SET_STRING_FUNC(device_date_time_),
        GET_STRING_FUNC(device_date_time_));

    addFieldWithExtractor(
        "total_energy_consumption_tariff_1",
        Quantity::Energy,
        DifVifKey("8E1003"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(1),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy consumption recorded by this meter on tariff 1.",
        SET_FUNC(total_energy_consumption_tariff_1_kwh_, Unit::KWH),
        GET_FUNC(total_energy_consumption_tariff_1_kwh_, Unit::KWH));

    addFieldWithExtractor(
        "total_energy_consumption_tariff_2",
        Quantity::Energy,
        DifVifKey("8E2003"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(2),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy consumption recorded by this meter on tariff 2.",
        SET_FUNC(total_energy_consumption_tariff_2_kwh_, Unit::KWH),
        GET_FUNC(total_energy_consumption_tariff_2_kwh_, Unit::KWH));


    addFieldWithExtractor(
        "total_energy_consumption_tariff_3",
        Quantity::Energy,
        DifVifKey("8E3003"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(3),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy consumption recorded by this meter on tariff 3.",
        SET_FUNC(total_energy_consumption_tariff_3_kwh_, Unit::KWH),
        GET_FUNC(total_energy_consumption_tariff_3_kwh_, Unit::KWH));

    addFieldWithExtractor(
        "total_energy_production_tariff_1",
        Quantity::Energy,
        DifVifKey("8E10833C"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(1),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy production recorded by this meter on tariff 1.",
        SET_FUNC(total_energy_production_tariff_1_kwh_, Unit::KWH),
        GET_FUNC(total_energy_production_tariff_1_kwh_, Unit::KWH));


    addFieldWithExtractor(
        "total_energy_production_tariff_2",
        Quantity::Energy,
        DifVifKey("8E20833C"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(2),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy production recorded by this meter on tariff 2.",
        SET_FUNC(total_energy_production_tariff_2_kwh_, Unit::KWH),
        GET_FUNC(total_energy_production_tariff_2_kwh_, Unit::KWH));


    addFieldWithExtractor(
        "total_energy_production_tariff_3",
        Quantity::Energy,
        DifVifKey("8E30833C"),
        VifScaling::Auto,
        MeasurementType::Unknown,
        ValueInformation::None,
        StorageNr(0),
        TariffNr(3),
        IndexNr(1),
        PrintProperty::JSON | PrintProperty::FIELD,
        "The total energy production recorded by this meter on tariff 3.",
        SET_FUNC(total_energy_production_tariff_3_kwh_, Unit::KWH),
        GET_FUNC(total_energy_production_tariff_3_kwh_, Unit::KWH));
}

// Test: MyElectricity1 amiplus 10101010 NOKEY
// telegram=|4E4401061010101002027A00004005|2F2F0E035040691500000B2B300300066D00790C7423400C78371204860BABC8FC100000000E833C8074000000000BAB3C0000000AFDC9FC0136022F2F2F2F2F|
// {"media":"electricity","meter":"amiplus","name":"MyElectricity1","id":"10101010","total_energy_consumption_kwh":15694.05,"current_power_consumption_kw":0.33,"total_energy_production_kwh":7.48,"current_power_production_kw":0,"voltage_at_phase_1_v":236,"voltage_at_phase_2_v":0,"voltage_at_phase_3_v":0,"device_date_time":"2019-03-20 12:57","total_energy_consumption_tariff_1_kwh":0,"total_energy_consumption_tariff_2_kwh":0,"total_energy_consumption_tariff_3_kwh":0,"total_energy_production_tariff_1_kwh":0,"total_energy_production_tariff_2_kwh":0,"total_energy_production_tariff_3_kwh":0,"timestamp":"1111-11-11T11:11:11Z"}
// |MyElectricity1;10101010;15694.050000;0.330000;7.480000;0.000000;236.000000;0.000000;0.000000;0.000000;0.000000;0.000000;0.000000;0.000000;0.000000;1111-11-11 11:11.11

// Test: MyElectricity2 amiplus 00254358 NOKEY
// amiplus/apator electricity meter with three phase voltages

// telegram=|5E44B6105843250000027A2A005005|2F2F0C7835221400066D404708AC2A400E032022650900000E833C0000000000001B2B9647000B2B5510000BAB3C0000000AFDC9FC0135020AFDC9FC0245020AFDC9FC0339020BABC8FC100000002F2F|
// {"media":"electricity","meter":"amiplus","name":"MyElectricity2","id":"00254358","total_energy_consumption_kwh":9652.22,"current_power_consumption_kw":1.055,"total_energy_production_kwh":0,"current_power_production_kw":0,"voltage_at_phase_1_v":235,"voltage_at_phase_2_v":245,"voltage_at_phase_3_v":239,"device_date_time":"2021-10-12 08:07","total_energy_consumption_tariff_1_kwh":0,"total_energy_consumption_tariff_2_kwh":0,"total_energy_consumption_tariff_3_kwh":0,"total_energy_production_tariff_1_kwh":0,"total_energy_production_tariff_2_kwh":0,"total_energy_production_tariff_3_kwh":0,"timestamp":"1111-11-11T11:11:11Z"}
// |MyElectricity2;00254358;9652.220000;1.055000;0.000000;0.000000;235.000000;245.000000;239.000000;0.000000;0.000000;0.000000;0.000000;0.000000;0.000000;1111-11-11 11:11.11

// Test: MyElectricity3 amiplus 86064864 NOKEY
// amiplus/apator electricity meter with three phase voltages and 2 tariffs

// telegram=|804401066448068602027A000070052F2F|066D1E5C11DA21400C78644806868E10036110012500008E20038106531800008E10833C9949000000008E20833C8606000000001B2B5228020B2B3217000BAB3C0000000AFDC9FC0131020AFDC9FC0225020AFDC9FC0331020BABC8FC100000002F2F2F2F2F2F2F2F2F2F2F2F2FDE47|
// {"media":"electricity","meter":"amiplus","name":"MyElectricity3","id":"86064864","total_energy_consumption_kwh":0,"current_power_consumption_kw":1.732,"total_energy_production_kwh":0,"current_power_production_kw":0,"voltage_at_phase_1_v":231,"voltage_at_phase_2_v":225,"voltage_at_phase_3_v":231,"device_date_time":"2022-01-26 17:28","total_energy_consumption_tariff_1_kwh":25011.061,"total_energy_consumption_tariff_2_kwh":18530.681,"total_energy_consumption_tariff_3_kwh":0,"total_energy_production_tariff_1_kwh":4.999,"total_energy_production_tariff_2_kwh":0.686,"total_energy_production_tariff_3_kwh":0,"timestamp":"1111-11-11T11:11:11Z"}
// |MyElectricity3;86064864;0.000000;1.732000;0.000000;0.000000;231.000000;225.000000;231.000000;25011.061000;18530.681000;0.000000;4.999000;0.686000;0.000000;1111-11-11 11:11.11
