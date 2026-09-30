//===- DependencyScanningUtils.cpp - Common Scanning Utilities ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/DependencyScanning/DependencyScanningUtils.h"

using namespace clang;
using namespace dependencies;

TranslationUnitDeps FullDependencyConsumer::takeTranslationUnitDeps() {
  TranslationUnitDeps TU;

  TU.ID.ContextHash = std::move(ContextHash);
  TU.ID.ModuleName = std::move(ModuleName);
  TU.NamedModuleDeps = std::move(NamedModuleDeps);
  TU.FileDeps = std::move(Dependencies);
  TU.PrebuiltModuleDeps = std::move(PrebuiltModuleDeps);
  TU.VisibleModules = std::move(VisibleModules);
  TU.Commands = std::move(Commands);
  TU.IncludeTreeID = std::move(IncludeTreeID);

  for (auto &&M : ClangModuleDeps) {
    auto &MD = M.second;
    // TODO: Avoid handleModuleDependency even being called for modules
    //   we've already seen.
    if (AlreadySeen.count(M.first))
      continue;
    TU.ModuleGraph.push_back(std::move(MD));
  }
  TU.ClangModuleDeps = std::move(DirectModuleDeps);

  return TU;
}

CallbackActionController::~CallbackActionController() {}

static void dumpModuleID(llvm::raw_ostream &OS, const ModuleID &ID, unsigned Indent) {
  for (unsigned I = 0; I < Indent; ++I) OS << ' ';
  OS << "{ Name=\"" << ID.ModuleName << "\", ContextHash=\"" << ID.ContextHash << "\" }";
}

void TranslationUnitDeps::dump() const { dump(llvm::errs()); }

void TranslationUnitDeps::dump(llvm::raw_ostream &OS) const {
  unsigned Indent = 0;
  auto IndentFn = [&](unsigned N) {
    for (unsigned I = 0; I < N; ++I) OS << ' ';
  };

  IndentFn(Indent); OS << "TranslationUnitDeps" << '\n';
  Indent += 2;

  // ID
  IndentFn(Indent); OS << "ID: ";
  dumpModuleID(OS, ID, 0);
  OS << '\n';

  // ModuleGraph
  IndentFn(Indent); OS << "ModuleGraph: [\n";
  for (const auto &MD : ModuleGraph) {
    // Reuse ModuleDeps::dump with increased indentation by capturing output to a temporary stream line-by-line.
    // As ModuleDeps::dump handles its own newlines and indentation from zero, we prefix indent for each line.
    std::string S;
    llvm::raw_string_ostream SS(S);
    MD.dump(SS);
    SS.flush();
    llvm::StringRef SR(S);
    while (!SR.empty()) {
      auto Line = SR.take_until([](char C){ return C=='\n'; });
      IndentFn(Indent + 2);
      OS << Line << '\n';
      SR = SR.drop_front(Line.size());
      if (SR.starts_with("\n")) SR = SR.drop_front(1);
    }
  }
  IndentFn(Indent); OS << "]\n";

  // FileDeps
  IndentFn(Indent); OS << "FileDeps: [\n";
  for (const auto &FD : FileDeps) {
    IndentFn(Indent + 2); OS << '"' << FD << '"' << '\n';
  }
  IndentFn(Indent); OS << "]\n";

  // PrebuiltModuleDeps
  IndentFn(Indent); OS << "PrebuiltModuleDeps: [\n";
  for (const auto &PM : PrebuiltModuleDeps) {
    IndentFn(Indent + 2);
    OS << "{ ModuleName=\"" << PM.ModuleName
       << "\", PCMFile=\"" << PM.PCMFile
       << "\", ModuleMapFile=\"" << PM.ModuleMapFile << "\" }\n";
  }
  IndentFn(Indent); OS << "]\n";

  // ClangModuleDeps
  IndentFn(Indent); OS << "ClangModuleDeps: [\n";
  for (const auto &MID : ClangModuleDeps) {
    IndentFn(Indent + 2);
    dumpModuleID(OS, MID, 0);
    OS << '\n';
  }
  IndentFn(Indent); OS << "]\n";

  // VisibleModules
  IndentFn(Indent); OS << "VisibleModules: [\n";
  for (const auto &VM : VisibleModules) {
    IndentFn(Indent + 2); OS << '"' << VM << '"' << '\n';
  }
  IndentFn(Indent); OS << "]\n";

  // NamedModuleDeps
  IndentFn(Indent); OS << "NamedModuleDeps: [\n";
  for (const auto &NM : NamedModuleDeps) {
    IndentFn(Indent + 2); OS << '"' << NM << '"' << '\n';
  }
  IndentFn(Indent); OS << "]\n";

  // Commands
  IndentFn(Indent); OS << "Commands: [\n";
  for (const auto &Cmd : Commands) {
    IndentFn(Indent + 2);
    OS << "{ Executable=\"" << Cmd.Executable << "\", Arguments=[";
    for (size_t I = 0; I < Cmd.Arguments.size(); ++I) {
      OS << '"' << Cmd.Arguments[I] << '"';
      if (I + 1 < Cmd.Arguments.size()) OS << ", ";
    }
    OS << "] }\n";
  }
  IndentFn(Indent); OS << "]\n";
}
