#ifndef PYRECODES_TEMPLATES_H
#define PYRECODES_TEMPLATES_H

/* *****************************************************************************
Copyright (c) 2016-2023, The Regents of the University of California (Regents).
All rights reserved. THE SOFTWARE IS PROVIDED "AS IS".
*************************************************************************** */

// Starter templates for the two files the pyrecodes UI builds. Embedded as
// string literals (rather than read from disk) so the "Load Template" buttons
// work regardless of the working directory and without any build-system change
// when these widgets are compiled into R2DTool. The same content is mirrored in
// pyrecodes_input_templates/template_*.json for users who want the files.
//
// The two templates form one coherent, minimal-but-complete, valid pyrecodes
// 0.3.0 community: a supplier + a building in one locality, with every system
// configuration section present (Constants, Content, DamageInput, Resources,
// ResilienceCalculator). Users edit from here instead of starting blank.

#include <QString>

namespace PyrecodesTemplates {

inline QString componentLibrary() {
  return QString::fromUtf8(R"({
    "ExampleSupplier": {
        "ComponentClass": {
            "FileName": "standard_irecodes_component",
            "ClassName": "StandardiReCoDeSComponent"
        },
        "RecoveryModel": {
            "FileName": "component_level_recovery_activities_model",
            "ClassName": "ComponentLevelRecoveryActivitiesModel",
            "Parameters": {
                "Repair": {
                    "Duration": { "Deterministic": { "Value": 10 } },
                    "Demand": [],
                    "PrecedingActivities": []
                }
            },
            "DamageFunctionalityRelation": { "Type": "ReverseBinary" }
        },
        "Supply": {
            "ElectricPower": {
                "Amount": 10,
                "FunctionalityToAmountRelation": "Linear",
                "UnmetDemandToAmountRelation": "Binary"
            }
        }
    },
    "ExampleBuilding": {
        "ComponentClass": {
            "FileName": "standard_irecodes_component",
            "ClassName": "StandardiReCoDeSComponent"
        },
        "RecoveryModel": {
            "FileName": "component_level_recovery_activities_model",
            "ClassName": "ComponentLevelRecoveryActivitiesModel",
            "Parameters": {
                "Repair": {
                    "Duration": { "Lognormal": { "Median": 30, "Dispersion": 0.4 } },
                    "Demand": [],
                    "PrecedingActivities": []
                }
            },
            "DamageFunctionalityRelation": { "Type": "ReverseBinary" }
        },
        "OperationDemand": {
            "ElectricPower": {
                "Amount": 1,
                "FunctionalityToAmountRelation": "Linear"
            }
        }
    }
})");
}

inline QString systemConfiguration() {
  return QString::fromUtf8(R"({
    "Constants": {
        "START_TIME_STEP": 0,
        "MAX_TIME_STEP": 100,
        "DISASTER_TIME_STEP": 1
    },
    "Content": {
        "Locality 1": {
            "Coordinates": { "Centroid": { "X": 0, "Y": 0 } },
            "Components": {
                "Infrastructure": [
                    {
                        "ElectricPowerSystem": {
                            "CreatorClassName": "JSONSubsystemCreator",
                            "CreatorFileName": "json_subsystem_creator",
                            "Parameters": { "ComponentsInLocality": { "ExampleSupplier": 1 } }
                        }
                    }
                ],
                "BuildingStock": [
                    {
                        "Buildings": {
                            "CreatorClassName": "JSONSubsystemCreator",
                            "CreatorFileName": "json_subsystem_creator",
                            "Parameters": { "ComponentsInLocality": { "ExampleBuilding": 1 } }
                        }
                    }
                ]
            }
        }
    },
    "DamageInput": {
        "FileName": "list_damage_input",
        "ClassName": "ListDamageInput",
        "Parameters": [ 0.5, 0.5 ]
    },
    "Resources": {
        "ElectricPower": {
            "Group": "Utilities",
            "DistributionModel": {
                "ClassName": "UtilityDistributionModel",
                "FileName": "utility_distribution_model",
                "Parameters": {
                    "DistributionPriority": {
                        "FileName": "component_based_priority",
                        "ClassName": "ComponentBasedPriority",
                        "Parameters": [ [ "ExampleSupplier", [ "Locality 1" ], "OperationDemand" ] ]
                    }
                }
            }
        }
    },
    "ResilienceCalculator": [
        {
            "FileName": "recodes_calculator",
            "ClassName": "ReCoDeSCalculator",
            "Parameters": { "Scope": "All", "Resources": [ "ElectricPower" ] }
        }
    ]
})");
}

} // namespace PyrecodesTemplates

#endif // PYRECODES_TEMPLATES_H
