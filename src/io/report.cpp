// Copyright 2017 Lybros.

#include "report.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QTextStream>

#include "theia/theia.h"

#include "../options.h"
#include "../project.h"
#include "../reconstructor.h"
#include "../storage.h"
#include "../theia_utils.h"

using theia::DescriptorExtractorType;

Report::Report(Project* project) : project_(project) {
  options_ = project_->GetOptions();
  storage_ = project_->GetStorage();
}

Report::~Report() {
}

QString Report::GetDefaultReportPath() {
  return QDir(storage_->GetOutputLocation()).filePath(
        QString("models/build_") +
        QDateTime::currentDateTime().toString(Qt::ISODate) +
        QString("_report.txt") );
}

bool Report::GenerateSmartReconstructionReport(QString filepath) {
  QFile report(filepath);
  if (!report.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
    return false;
  }

  QTextStream r(&report);
  r << "Smart reconstruction report." << endl
    << project_->GetProjectName() << endl
    << QDateTime::currentDateTime().toString() << endl
    << QProcessEnvironment::systemEnvironment().value(
         HARDWARE_INFO_VARIABLE, "no hardware data provided.") << endl;
  r << endl;

  r << "----- OVERVIEW -----" << endl
    << "project name:     " << project_->GetProjectName() << endl
    << "project location: " << project_->GetProjectPath() << endl
    << "dataset:          " << project_->GetImagesPath() << endl
    << "dataset size:     " << storage_->NumberOfImages() << endl
    << "output location:  " << project_->GetDefaultOutputPath() << endl;
  r << endl;

  r << "----- FEATURE EXTRACTION -----" << endl;
  if (using_prebuilt_matches_) {
    r << "extraction was skipped" << endl;
  } else {
    r << "extraction was performed." << endl
      << "desriptor type: "
      << DescriptorExtractorTypeToString(options_->descriptor_type_) << endl;
  }
  r << endl;

  r << "----- FEATURE MATCHING -----" << endl;
  if (using_prebuilt_matches_) {
    r << "feature matching was skipped (read from filesystem)" << endl
      << "timing (reading matches): " << reading_matches_time_ << "s" << endl;
  } else {
    r << "feature matching was performed." << endl
      << "matching strategy: "
      << MatchingStrategyToString(options_->match_strategy_) << endl
      << "timing (extract&match): " << extraction_matching_time_ << "s" << endl;
  }
  r << endl;

  r << "----- RECONSTRUCTION -----" << endl;
  r << "shared calibration: "
    << (shared_calibration_ ? "applied" : "not applied") << endl
    << "prior camera intrinsics: "
    << (using_camera_intrinsics_prior_ ? "from file" : "from exif") << endl;
  r << "timing:" << endl
    << "  overall:        " << overall_time_ << "s" << endl
    << "  reconstruction: " << reconstruction_time_ << "s" << endl
    << "  colorizing:     " << colorizing_time_ << "s" << endl;
  r << endl;

  r << "Hope it was helpful!" << endl;

  report.close();
  return true;
}
