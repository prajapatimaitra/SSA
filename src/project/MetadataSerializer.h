#pragma once

#include "project/ProjectMetadata.h"
#include <QString>

namespace ssa::project {

class MetadataSerializer {
public:
    static QString serialize(const ProjectMetadata& metadata);
    static std::optional<ProjectMetadata> deserialize(const QString& jsonString);
};

} // namespace ssa::project
