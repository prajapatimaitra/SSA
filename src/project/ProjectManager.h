#pragma once

#include "project/ProjectMetadata.h"
#include <optional>
#include <string>

namespace ssa::project {

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager() = default;

    // Creates the .ssa directory bundle at the specified path
    bool createNewProject(const std::string& path, int videoWidth, int videoHeight);

    // Creates an .ssa project bundle from an external media file (copies media into bundle)
    bool createProjectFromExternalMedia(const std::string& sourceMediaPath, const std::string& destBundlePath);

    // Duplicates an existing .ssa project bundle non-destructively using hard links
    bool duplicateProject(const std::string& sourceBundlePath, const std::string& newBundlePath);

    // Saves the metadata to metadata.json inside the bundle
    bool saveMetadata(const ProjectMetadata& metadata);

    // Loads metadata from metadata.json inside the current bundle
    std::optional<ProjectMetadata> loadMetadata();

    const std::string& getCurrentProjectPath() const { return m_currentProjectPath; }

private:
    std::string m_currentProjectPath;
};

} // namespace ssa::project
