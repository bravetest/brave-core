// Copyright 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import BraveCore
import Data
import Foundation
import Preferences
import os

/// A class that helps manage file sources and enabled filter lists so that 2 engines (standard and aggressive) are continually updated.
@MainActor public class AdBlockGroupsManager {
  @MainActor protocol SourceProvider {
    /// All the enabled sources.
    /// They will be compiled in the order given so ensure the order here corresponds with how you want your engines to look
    var enabledSources: [GroupedAdBlockEngine.Source] { get }

    /// If we didn't load the main engine from cache we need to load using the old cache mechanism
    /// This is only temporary so we're not left with no ad-block during the upgrade.
    /// We can drop all of this in future upgrades as by then we will have files cached in the new format
    func legacyCacheFiles(
      for engineType: GroupedAdBlockEngine.EngineType
    ) -> [AdBlockEngineManager.FileInfo]

    func sources(
      for engineType: GroupedAdBlockEngine.EngineType
    ) -> [GroupedAdBlockEngine.Source]
  }

  typealias CosmeticFilterModelTuple = (isAlwaysAggressive: Bool, model: CosmeticFilterModel)
  public static let shared = AdBlockGroupsManager(
    standardManager: GroupedAdBlockEngine.EngineType.standard.makeDefaultManager(),
    aggressiveManager: GroupedAdBlockEngine.EngineType.aggressive.makeDefaultManager(),
    contentBlockerManager: ContentBlockerManager(),
    sourceProvider: DefaultSourceProvider()
  )

  let standardManager: AdBlockEngineManager
  let aggressiveManager: AdBlockEngineManager
  let sourceProvider: SourceProvider
  /// The info for the resource file. This is a shared file used by all filter lists that contain scriplets. This information is used for lazy loading.
  public var resourcesInfo: GroupedAdBlockEngine.ResourcesInfo?
  /// The content blocker manager that will be used to compile and manage content blockers
  let contentBlockerManager: ContentBlockerManager

  init(
    standardManager: AdBlockEngineManager,
    aggressiveManager: AdBlockEngineManager,
    contentBlockerManager: ContentBlockerManager,
    sourceProvider: SourceProvider
  ) {
    self.standardManager = standardManager
    self.aggressiveManager = aggressiveManager
    self.contentBlockerManager = contentBlockerManager
    self.sourceProvider = sourceProvider
    self.resourcesInfo = nil
  }

  /// Tells us if we have an engine available for the given engine type
  func hasEngine(for engineType: GroupedAdBlockEngine.EngineType) -> Bool {
    return getManager(for: engineType).engine != nil
  }

  /// Handle memory warnings by freeing up some memory
  func didReceiveMemoryWarning() async {
    await standardManager.engine?.clearCaches()
    await aggressiveManager.engine?.clearCaches()
  }

  /// Load any cache data so its ready right during launch
  func loadResourcesFromCache() async {
    if let resourcesInfo = getCachedResourcesInfo() {
      // We need this for all filter lists so we can't compile anything until we download it
      self.resourcesInfo = resourcesInfo

      ContentBlockerManager.log.debug(
        "Loaded resources component from cache: `\(resourcesInfo.localFileURL.path(percentEncoded: false))`"
      )
    }
  }

  /// This will load bundled data for the given content blocking modes. But only if the files are not already compiled.
  func loadBundledDataIfNeeded() async {
    // Compile bundled blocklists but only if we don't have anything already loaded.
    await ContentBlockerManager.GenericBlocklistType.allCases.asyncConcurrentForEach {
      genericType in
      let blocklistType = ContentBlockerManager.BlocklistType.generic(genericType)
      let missingModes = await self.contentBlockerManager.missingModes(
        for: blocklistType,
        version: genericType.version
      )

      do {
        try await self.contentBlockerManager.compileBundledRuleList(
          for: genericType,
          modes: missingModes
        )
      } catch {
        assertionFailure("A bundled file should not fail to compile")
      }
    }
  }

  /// Load the engines from cache for all engine types
  /// Will use the cached dat files to do this, however, for upgrades, it
  /// will load from legacy storage (using text files) if the dat file is unavailable.
  func loadEnginesFromCache() async {
    await GroupedAdBlockEngine.EngineType.allCases.asyncConcurrentForEach { engineType in
      await self.loadEngineFromCache(for: engineType)
    }
  }

  /// Load the engine from cache for the given engine type
  /// Will use the cached dat files to do this, however, for upgrades, it
  /// will load from legacy storage (using text files) if the dat file is unavailable.
  private func loadEngineFromCache(for engineType: GroupedAdBlockEngine.EngineType) async {
    let manager = getManager(for: engineType)

    if await !manager.loadFromCache(resourcesInfo: self.resourcesInfo) {
      // This migration will add ~24s on an iPhone 8 (~8s on an iPhone 14)
      // Even though its a one time thing, let's skip it.
      // We never waited for the aggressive engines to be ready before anyways
      guard engineType == .standard else { return }
      for fileInfo in sourceProvider.legacyCacheFiles(for: engineType) {
        manager.add(fileInfo: fileInfo)
      }

      await manager.compileImmediatelyIfNeeded(
        for: sourceProvider.enabledSources,
        resourcesInfo: self.resourcesInfo,
        contentBlockerManager: contentBlockerManager
      )
    }
  }

  /// Inform this manager of updates to the resources so our engines can be updated
  func didUpdateResourcesComponent(resourcesFileURL: URL) {
    let folderSubPath = AdblockService.extractRelativePath(fromComponentURL: resourcesFileURL)
    Preferences.AppState.lastAdBlockResourcesFilePath.value = folderSubPath
    Preferences.AppState.lastAdBlockResourcesFolderPath.value = nil
    let resourcesInfo = getResourcesInfo(fromFileURL: resourcesFileURL)
    updateIfNeeded(resourcesInfo: resourcesInfo)
  }

  /// Update the file managers with the latest files and start a delayed task to compile the engines.
  /// - Parameters:
  ///   - fileInfos: The file infos to update on the appropriate engine manager
  func update(
    fileInfos: [AdBlockEngineManager.FileInfo]
  ) {
    let enabledSources = sourceProvider.enabledSources

    for engineType in GroupedAdBlockEngine.EngineType.allCases {
      let manager = getManager(for: engineType)
      let sources = sourceProvider.sources(for: engineType)
      var updatedFiles = false

      // Compile content blockers if this filter list is enabled
      for fileInfo in fileInfos {
        guard sources.contains(fileInfo.filterListInfo.source) else {
          // This file is not for this engine type
          continue
        }

        if enabledSources.contains(fileInfo.filterListInfo.source) {
          Task {
            await ensureContentBlockers(for: fileInfo, engineType: engineType)
          }
        }

        updatedFiles = true
        manager.add(fileInfo: fileInfo)
      }

      if updatedFiles {
        manager.compileDelayedIfNeeded(
          for: enabledSources,
          resourcesInfo: resourcesInfo,
          contentBlockerManager: contentBlockerManager
        )
      }
    }
  }

  /// Update the file managers with the latest files and will compile the engines right away.
  /// - Parameters:
  ///   - fileInfos: The file infos to update on the appropriate engine manager
  func updateImmediately(
    fileInfos: [AdBlockEngineManager.FileInfo]
  ) async {
    let enabledSources = sourceProvider.enabledSources

    for engineType in GroupedAdBlockEngine.EngineType.allCases {
      let manager = getManager(for: engineType)
      let sources = sourceProvider.sources(for: engineType)
      var updatedFiles = false

      // Compile content blockers if this filter list is enabled
      for fileInfo in fileInfos {
        guard sources.contains(fileInfo.filterListInfo.source) else {
          // This file is not for this engine type
          continue
        }

        if enabledSources.contains(fileInfo.filterListInfo.source) {
          await ensureContentBlockers(for: fileInfo, engineType: engineType)
        }

        updatedFiles = true
        manager.add(fileInfo: fileInfo)
      }

      if updatedFiles {
        await manager.compileImmediatelyIfNeeded(
          for: enabledSources,
          resourcesInfo: resourcesInfo,
          contentBlockerManager: contentBlockerManager
        )
      }
    }
  }

  /// Handle updated filter list info. Will compile the engine immediately.
  /// - Parameters:
  ///   - fileInfo: The file info to update on the appropriate engine manager
  func updateImmediately(
    fileInfo: AdBlockEngineManager.FileInfo
  ) async {
    await updateImmediately(fileInfos: [fileInfo])
  }

  /// Handle updated filter list info
  /// - Parameters:
  ///   - fileInfo: The file info to update on the appropriate engine manager
  func update(
    fileInfo: AdBlockEngineManager.FileInfo
  ) {
    update(fileInfos: [fileInfo])
  }

  /// Remove the file info from the list that is no longer available and compile the engines if it is needed.
  func removeFileInfo(
    for source: GroupedAdBlockEngine.Source
  ) {
    removeFileInfos(for: [source])
  }

  /// Remove the file info from the list that is no longer available and compile the engines if it is needed.
  func removeFileInfoImmediately(
    for source: GroupedAdBlockEngine.Source
  ) async {
    await removeFileInfosImmediately(for: [source])
  }

  /// Remove the file infos from the list that is no longer available and compile the engines if it is needed.
  func removeFileInfos(
    for sources: [GroupedAdBlockEngine.Source]
  ) {
    for engineType in GroupedAdBlockEngine.EngineType.allCases {
      let manager = getManager(for: engineType)
      for source in sources {
        manager.removeInfo(for: source)
      }

      manager.compileDelayedIfNeeded(
        for: sourceProvider.enabledSources,
        resourcesInfo: resourcesInfo,
        contentBlockerManager: contentBlockerManager
      )
    }
  }

  /// Remove the file infos from the list that is no longer available and compile the engines if it is needed.
  func removeFileInfosImmediately(
    for sources: [GroupedAdBlockEngine.Source]
  ) async {
    for engineType in GroupedAdBlockEngine.EngineType.allCases {
      let manager = getManager(for: engineType)
      for source in sources {
        manager.removeInfo(for: source)
      }

      await manager.compileImmediatelyIfNeeded(
        for: sourceProvider.enabledSources,
        resourcesInfo: resourcesInfo,
        contentBlockerManager: contentBlockerManager
      )
    }
  }

  /// Immediately compile any engines that have all the files ready.
  /// Will not compile anything is there is already the same set of files being compiled.
  func compileEnginesIfFilesAreReady() {
    for engineType in GroupedAdBlockEngine.EngineType.allCases {
      compileEngineIfFilesAreReady(for: engineType)
    }
  }

  /// Immediately compile the engine for the given type if it has all the files ready..
  /// Will not compile anything is there is already the same set of files being compiled.
  func compileEngineIfFilesAreReady(for engineType: GroupedAdBlockEngine.EngineType) {
    let manager = self.getManager(for: engineType)
    let enabledSources = sourceProvider.enabledSources(for: engineType)
    let availableSources = manager.compilableFiles(for: enabledSources)
      .map({ $0.filterListInfo.source })
    guard enabledSources.allSatisfy({ availableSources.contains($0) }) else {
      return
    }

    Task {
      await manager.compileImmediatelyIfNeeded(
        for: enabledSources,
        resourcesInfo: self.resourcesInfo,
        contentBlockerManager: contentBlockerManager
      )
    }
  }

  /// Ensure all engines and content blockers are compiled right away.
  /// Will not compile anything is there is already the same set of files being compiled.
  func compileEnginesIfNeeded() async {
    await GroupedAdBlockEngine.EngineType.allCases.asyncConcurrentForEach { engineType in
      let enabledSources = self.sourceProvider.enabledSources(for: engineType)
      let manager = self.getManager(for: engineType)
      await manager.compileImmediatelyIfNeeded(
        for: enabledSources,
        resourcesInfo: self.resourcesInfo,
        contentBlockerManager: self.contentBlockerManager
      )
    }
  }

  /// Get all required rule lists for the given domain
  public func ruleLists(for domain: Domain) async -> Set<WKContentRuleList> {
    let validBlocklistTypes = self.validBlocklistTypes(for: domain)
    let level = domain.blockAdsAndTrackingLevel

    return await Set(
      validBlocklistTypes.asyncConcurrentCompactMap({ blocklistType -> WKContentRuleList? in
        let mode = blocklistType.mode(isAggressiveMode: level.isAggressive)

        do {
          return try await self.contentBlockerManager.ruleList(for: blocklistType, mode: mode)
        } catch {
          // We can't log the error because some rules have empty rules. This is normal
          // But on relaunches we try to reload the filter list and this will give us an error.
          // Need to find a more graceful way of handling this so error here can be logged properly
          return nil
        }
      })
    )
  }

  /// A list of all valid (enabled) blocklist types for the given domain
  private func validBlocklistTypes(for domain: Domain) -> Set<(ContentBlockerManager.BlocklistType)>
  {
    guard !domain.areAllShieldsOff else { return [] }

    // 1. Get the generic types
    let genericTypes = contentBlockerManager.validGenericTypes(for: domain)
    let genericRuleLists = genericTypes.map { genericType -> ContentBlockerManager.BlocklistType in
      return .generic(genericType)
    }

    guard domain.isShieldExpected(.adblockAndTp, considerAllShieldsOption: true) else {
      return Set(genericRuleLists)
    }

    let sourceBlocklistTypes = GroupedAdBlockEngine.EngineType.allCases.flatMap {
      engineType -> [ContentBlockerManager.BlocklistType] in
      return sourceProvider.enabledBlocklistTypes(for: engineType)
    }

    // 2. Get the sources types
    return Set(genericRuleLists).union(sourceBlocklistTypes)
  }

  /// Remove all un-needed content blockers
  public func cleaupInvalidRuleLists() async {
    let engineGroupTypes = GroupedAdBlockEngine.EngineType.allCases
      .flatMap({ engineType -> [ContentBlockerManager.BlocklistType] in
        return sourceProvider.blocklistTypes(for: engineType)
      })
    let allBlocklistTypes = ContentBlockerManager.BlocklistType.allStaticTypes
      .union(engineGroupTypes)
    await contentBlockerManager.cleaupInvalidRuleLists(validTypes: allBlocklistTypes)
  }

  /// Ensure all the content blockers are compiled for any file info found in the list of enabled sources
  private func ensureContentBlockers(
    for enabledSources: [GroupedAdBlockEngine.Source],
    engineType: GroupedAdBlockEngine.EngineType
  ) {
    let manager = getManager(for: engineType)
    // Compile all content blockers for the given manager
    manager.compilableFiles(for: enabledSources).forEach { fileInfo in
      Task {
        await ensureContentBlockers(for: fileInfo, engineType: engineType)
      }
    }
  }

  /// Ensure the content blocker is compiled for the given source the engine type supports it
  private func ensureContentBlockers(
    for fileInfo: AdBlockEngineManager.FileInfo,
    engineType: GroupedAdBlockEngine.EngineType
  ) async {
    guard
      let blocklistType = fileInfo.filterListInfo.source.blocklistType(
        engineType: engineType
      )
    else {
      return
    }

    let modes = await contentBlockerManager.missingModes(
      for: blocklistType,
      version: fileInfo.filterListInfo.version
    )

    await contentBlockerManager.compileRuleList(
      at: fileInfo.localFileURL,
      for: blocklistType,
      version: fileInfo.filterListInfo.version,
      modes: modes
    )
  }

  private func getCachedResourcesInfo() -> GroupedAdBlockEngine.ResourcesInfo? {
    if let resourcesFolderURL = AdblockService.makeAbsoluteURL(
      forComponentPath: Preferences.AppState.lastAdBlockResourcesFolderPath.value
    ), FileManager.default.fileExists(atPath: resourcesFolderURL.path) {
      // This is a legacy storage when we were gettting the component folder URL not the file URL
      let resourcesFileURL = resourcesFolderURL.appendingPathComponent(
        "resources.json",
        conformingTo: .json
      )
      return getResourcesInfo(fromFileURL: resourcesFileURL)
    } else if let resourcesFileURL = AdblockService.makeAbsoluteURL(
      forComponentPath: Preferences.AppState.lastAdBlockResourcesFilePath.value
    ), FileManager.default.fileExists(atPath: resourcesFileURL.path) {
      return getResourcesInfo(fromFileURL: resourcesFileURL)
    } else {
      return nil
    }
  }

  /// Convert the given folder URL to a `ResourcesInfo` object
  private func getResourcesInfo(fromFileURL fileURL: URL) -> GroupedAdBlockEngine.ResourcesInfo {
    return GroupedAdBlockEngine.ResourcesInfo(
      localFileURL: fileURL,
      version: fileURL.deletingLastPathComponent().lastPathComponent
    )
  }

  /// Add or update `resourcesInfo` if it is a newer version. This information is used for lazy loading.
  func updateIfNeeded(resourcesInfo: GroupedAdBlockEngine.ResourcesInfo) {
    guard self.resourcesInfo == nil || resourcesInfo.version > self.resourcesInfo!.version else {
      return
    }
    self.resourcesInfo = resourcesInfo

    GroupedAdBlockEngine.EngineType.allCases.forEach { engineType in
      let manager = self.getManager(for: engineType)

      Task {
        await manager.update(resourcesInfo: resourcesInfo)
      }
    }

    ContentBlockerManager.log.debug(
      "Updated resources component: `\(resourcesInfo.localFileURL.path(percentEncoded: false))`"
    )
  }

  /// Checks the general and regional engines to see if the request should be blocked
  func shouldBlock(
    requestURL: URL,
    sourceURL: URL,
    resourceType: AdblockEngine.ResourceType,
    domain: Domain
  ) async -> Bool {
    return await cachedEngines(for: domain).asyncConcurrentMap({ cachedEngine in
      return await cachedEngine.shouldBlock(
        requestURL: requestURL,
        sourceURL: sourceURL,
        resourceType: resourceType,
        isAggressiveMode: domain.blockAdsAndTrackingLevel.isAggressive
      )
    }).contains(where: { $0 })
  }

  /// This returns all the user script types for the given frame
  func makeEngineScriptTypes(
    frameURL: URL,
    isMainFrame: Bool,
    isDeAmpEnabled: Bool,
    domain: Domain
  ) async -> Set<UserScriptType> {
    // Add any engine scripts for this frame
    return await cachedEngines(for: domain).enumerated().asyncMap({
      index,
      cachedEngine -> Set<UserScriptType> in
      do {
        return try await cachedEngine.makeEngineScriptTypes(
          frameURL: frameURL,
          isMainFrame: isMainFrame,
          isDeAmpEnabled: isDeAmpEnabled,
          index: index
        )
      } catch {
        assertionFailure()
        return []
      }
    }).reduce(
      Set<UserScriptType>(),
      { partialResult, scriptTypes in
        return partialResult.union(scriptTypes)
      }
    )
  }

  /// Returns all appropriate engines for the given domain
  func cachedEngines(for domain: Domain) -> [GroupedAdBlockEngine] {
    guard domain.isShieldExpected(.adblockAndTp, considerAllShieldsOption: true) else { return [] }
    return GroupedAdBlockEngine.EngineType.allCases.compactMap({ getManager(for: $0).engine })
  }

  /// Returns all the models for this frame URL
  func cosmeticFilterModels(
    forFrameURL frameURL: URL,
    domain: Domain
  ) async -> [CosmeticFilterModelTuple] {
    return await cachedEngines(for: domain).asyncConcurrentCompactMap {
      cachedEngine -> CosmeticFilterModelTuple? in
      do {
        guard let model = try await cachedEngine.cosmeticFilterModel(forFrameURL: frameURL) else {
          return nil
        }
        return (cachedEngine.type.isAlwaysAggressive, model)
      } catch {
        assertionFailure()
        return nil
      }
    }
  }

  /// Get the appropriate manager for the given engine type.
  func getManager(for engineType: GroupedAdBlockEngine.EngineType) -> AdBlockEngineManager {
    switch engineType {
    case .standard: return standardManager
    case .aggressive: return aggressiveManager
    }
  }
}

extension GroupedAdBlockEngine.EngineType {
  fileprivate var defaultCachedFolderName: String {
    switch self {
    case .standard: return "standard"
    case .aggressive: return "aggressive"
    }
  }

  @MainActor fileprivate func makeDefaultManager() -> AdBlockEngineManager {
    return AdBlockEngineManager(engineType: self, cacheFolderName: defaultCachedFolderName)
  }
}

extension FilterListSetting {
  @MainActor var engineSource: GroupedAdBlockEngine.Source? {
    guard let componentId = componentId else { return nil }
    return .filterList(componentId: componentId)
  }
}

extension FilterList {
  @MainActor var engineSource: GroupedAdBlockEngine.Source {
    return .filterList(componentId: entry.componentId)
  }
}

extension AdBlockEngineManager.FileInfo {
  fileprivate init?(
    for source: GroupedAdBlockEngine.Source,
    downloadedFolderURL: URL
  ) {
    let version = downloadedFolderURL.lastPathComponent
    let localFileURL = downloadedFolderURL.appendingPathComponent("list.txt")

    guard FileManager.default.fileExists(atPath: localFileURL.relativePath) else {
      // We are loading the old component from cache. We don't want this file to be loaded.
      // When we download the new component shortly we will update our cache.
      // This should only trigger after an app update and eventually this check can be removed.
      return nil
    }

    self.init(
      filterListInfo: GroupedAdBlockEngine.FilterListInfo(
        source: source,
        version: version
      ),
      localFileURL: localFileURL
    )
  }
}

@MainActor class DefaultSourceProvider: AdBlockGroupsManager.SourceProvider {
  /// Return an array of all sources that are enabled according to user's settings
  /// - Note: This does not take into account the domain or global adblock toggle
  var enabledSources: [GroupedAdBlockEngine.Source] {
    var enabledSources = FilterListStorage.shared.enabledSources
    enabledSources.append(contentsOf: CustomFilterListStorage.shared.enabledSources)
    return enabledSources
  }

  /// Return an array of all sources that are enabled according to user's settings and for the given engine type
  /// - Note: This does not take into account the domain or global adblock toggle
  func sources(
    for engineType: GroupedAdBlockEngine.EngineType
  ) -> [GroupedAdBlockEngine.Source] {
    switch engineType {
    case .aggressive:
      var sources = FilterListStorage.shared.sources(for: engineType)
      sources.append(contentsOf: CustomFilterListStorage.shared.allSources)
      return sources
    case .standard:
      return FilterListStorage.shared.sources(for: engineType)
    }
  }

  func legacyCacheFiles(
    for engineType: GroupedAdBlockEngine.EngineType
  ) -> [AdBlockEngineManager.FileInfo] {
    return FilterListStorage.shared.allFilterListSettings
      .filter({ $0.isAlwaysAggressive == engineType.isAlwaysAggressive })
      .sorted(by: { $0.order?.intValue ?? 0 <= $1.order?.intValue ?? 0 })
      .compactMap({ setting in
        guard let source = setting.engineSource else { return nil }
        guard let folderURL = AdblockService.makeAbsoluteURL(forComponentPath: setting.folderPath)
        else { return nil }
        return AdBlockEngineManager.FileInfo(for: source, downloadedFolderURL: folderURL)
      })
  }
}

extension AdBlockGroupsManager.SourceProvider {
  /// Get all enabled sources for the given engine type
  func enabledSources(
    for engineType: GroupedAdBlockEngine.EngineType
  ) -> [GroupedAdBlockEngine.Source] {
    let enabledSources = self.enabledSources
    let sources = self.sources(for: engineType)
    return sources.filter({ enabledSources.contains($0) })
  }

  /// Get all enabled blocklist types for the given engine type
  func enabledBlocklistTypes(
    for engineType: GroupedAdBlockEngine.EngineType
  ) -> [ContentBlockerManager.BlocklistType] {
    return enabledSources(for: engineType).compactMap { source in
      return source.blocklistType(engineType: engineType)
    }
  }

  /// Get all valid blocklist types for the given engine type
  func blocklistTypes(
    for engineType: GroupedAdBlockEngine.EngineType
  ) -> [ContentBlockerManager.BlocklistType] {
    return sources(for: engineType).compactMap { source in
      return source.blocklistType(engineType: engineType)
    }
  }
}
