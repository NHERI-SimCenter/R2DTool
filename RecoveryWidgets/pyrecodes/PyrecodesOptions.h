#ifndef PYRECODES_OPTIONS_H
#define PYRECODES_OPTIONS_H

/* *****************************************************************************
Copyright (c) 2016-2023, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THE SOFTWARE IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*************************************************************************** */

// Single source of truth for the option lists used by the pyrecodes UI
// drop-downs. Mirrors the classes registered in pyrecodes 0.3.0. Each
// ClassName is paired with its FileName (the snake_case python module that
// pyrecodes imports it from); the UI auto-fills FileName when the user picks a
// ClassName, so the produced JSON matches what pyrecodes expects.

#include <QStringList>
#include <QMap>
#include <QString>

namespace PyrecodesOptions {

// ---- Component classes (ComponentClass.ClassName) -------------------------
inline QStringList componentClasses() {
  return QStringList()
    << "StandardiReCoDeSComponent"
    << "BuildingWithEmergencyCalls"
    << "InfrastructureInterface"
    << "Bridge"
    << "Roadway"
    << "Tunnel"
    << "R2DBuilding"
    << "R2DBuildingWithHouseholds"
    << "R2DTown"
    << "R2DBridge"
    << "R2DRoadway"
    << "R2DTunnel"
    << "R2DPipe"
    << "R2DTransportationComponent";
}

// ---- Recovery model classes (RecoveryModel.ClassName) ---------------------
inline QStringList recoveryModelClasses() {
  return QStringList()
    << "NoRecoveryActivityModel"
    << "ComponentLevelRecoveryActivitiesModel"
    << "InfrastructureInterfaceRecoveryModel";
}

// ---- Relation types -------------------------------------------------------
// Used for Supply.FunctionalityToAmountRelation, Supply.UnmetDemandToAmountRelation,
// OperationDemand.FunctionalityToAmountRelation and
// RecoveryModel.DamageFunctionalityRelation.Type.
inline QStringList relationTypes() {
  return QStringList()
    << "Constant"
    << "Linear"
    << "ReverseLinear"
    << "Binary"
    << "ReverseBinary"
    << "MultipleStep";
}

// ---- Probability distributions (recovery activity Duration) ---------------
inline QStringList probabilityDistributions() {
  return QStringList()
    << "Deterministic"
    << "Lognormal";
}

// ---- Resource groups ------------------------------------------------------
inline QStringList resourceGroups() {
  return QStringList()
    << "Utilities"
    << "TransferService"
    << "RecoveryResources"
    << "BridgeService";
}

// ---- Resource distribution model classes ----------------------------------
inline QStringList distributionModelClasses() {
  return QStringList()
    << "UtilityDistributionModel"
    << "HousingDistributionModel"
    << "TransferServiceDistributionModelPotentialPaths"
    << "BridgeServiceDistributionModel"
    << "REWETDistributionModel"
    << "ResidualDemandTrafficDistributionModel";
}

// ---- Distribution priority classes ----------------------------------------
inline QStringList distributionPriorityClasses() {
  return QStringList()
    << "ComponentBasedPriority"
    << "ComponentTypePriority"
    << "SupplierOnlyPriority"
    << "RandomPriority"
    << "RandomPriorityWithPrioritizedInterfaces";
}

// ---- Damage input classes -------------------------------------------------
inline QStringList damageInputClasses() {
  return QStringList()
    << "R2DDamageInput"
    << "ListDamageInput"
    << "FileDamageInput";
}

// ---- Resilience calculator classes ----------------------------------------
inline QStringList resilienceCalculatorClasses() {
  return QStringList()
    << "ReCoDeSCalculator"
    << "NISTGoalsCalculator"
    << "ComponentRecoveryTimeCalculator"
    << "R2DComponentRecoveryTimeCalculator"
    << "FullRecoveryTimeCalculator";
}

// ---- Subsystem creator classes --------------------------------------------
inline QStringList subsystemCreatorClasses() {
  return QStringList()
    << "JSONSubsystemCreator"
    << "R2DSubsystemCreator"
    << "R2DSubsystemCreatorWithHouseholds"
    << "RecoveryResourceSuppliersCreator"
    << "Tier1InfrastructureCreator";
}

// ---- ClassName -> FileName (python module) map ----------------------------
// Auto-fills the FileName field that pyrecodes 0.3.0 requires alongside each
// ClassName. Returns an empty string for unknown classes (caller may leave the
// FileName out / let the user type one).
inline QString fileNameForClass(const QString &className) {
  static const QMap<QString, QString> map = {
    // component classes
    {"StandardiReCoDeSComponent",        "standard_irecodes_component"},
    {"BuildingWithEmergencyCalls",       "building_with_emergency_calls"},
    {"InfrastructureInterface",          "infrastructure_interface"},
    {"Bridge",                           "component"},
    {"Roadway",                          "component"},
    {"Tunnel",                           "component"},
    {"R2DBuilding",                      "r2d_component"},
    {"R2DBridge",                        "r2d_component"},
    {"R2DRoadway",                       "r2d_component"},
    {"R2DTunnel",                        "r2d_component"},
    {"R2DPipe",                          "r2d_component"},
    {"R2DTransportationComponent",       "r2d_component"},
    {"R2DComponent",                     "r2d_component"},
    {"R2DBuildingWithHouseholds",        "r2d_building_with_households"},
    {"R2DTown",                          "r2d_building_with_households"},
    // recovery models
    {"NoRecoveryActivityModel",                  "no_recovery_activity_model"},
    {"ComponentLevelRecoveryActivitiesModel",    "component_level_recovery_activities_model"},
    {"InfrastructureInterfaceRecoveryModel",     "infrastructure_interface_recovery_model"},
    // distribution models
    {"UtilityDistributionModel",                       "utility_distribution_model"},
    {"HousingDistributionModel",                       "housing_distribution_model"},
    {"TransferServiceDistributionModelPotentialPaths", "transfer_service_distribution_model_potential_paths"},
    {"BridgeServiceDistributionModel",                 "bridge_service_distribution_model"},
    {"REWETDistributionModel",                         "rewet_distribution_model"},
    {"ResidualDemandTrafficDistributionModel",         "residual_demand_traffic_distribution_model"},
    // distribution priorities
    {"ComponentBasedPriority",                   "component_based_priority"},
    {"ComponentTypePriority",                    "component_type_priority"},
    {"SupplierOnlyPriority",                     "supplier_only_priority"},
    {"RandomPriority",                           "random_priority"},
    {"RandomPriorityWithPrioritizedInterfaces",  "random_priority_with_prioritized_interfaces"},
    // damage input
    {"R2DDamageInput",                           "r2d_damage_input"},
    {"ListDamageInput",                          "list_damage_input"},
    {"FileDamageInput",                          "file_damage_input"},
    // resilience calculators
    {"ReCoDeSCalculator",                        "recodes_calculator"},
    {"NISTGoalsCalculator",                      "nist_goals_calculator"},
    {"ComponentRecoveryTimeCalculator",          "component_recovery_time_calculator"},
    {"R2DComponentRecoveryTimeCalculator",       "r2d_component_recovery_time_calculator"},
    {"FullRecoveryTimeCalculator",               "full_recovery_time_calculator"},
    // subsystem creators
    {"JSONSubsystemCreator",                     "json_subsystem_creator"},
    {"R2DSubsystemCreator",                      "r2d_subsystem_creator"},
    {"R2DSubsystemCreatorWithHouseholds",        "r2d_subsystem_creator"},
    {"RecoveryResourceSuppliersCreator",         "recovery_resource_suppliers_creator"},
    {"Tier1InfrastructureCreator",               "tier1_infrastructure_creator"},
  };
  return map.value(className, QString());
}

} // namespace PyrecodesOptions

#endif // PYRECODES_OPTIONS_H
