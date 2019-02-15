/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2017 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 * This example shows how to read and dump a DICOMDIR File
 *
 * Thanks:
 *   Tom Marynowski (lordglub gmail) for contributing the original 
 *    ReadAndDumpDICOMDIR.cxx example
 *   Mihail Isakov for contributing offset calculation code here:
 *    https://sourceforge.net/p/gdcm/mailman/gdcm-developers/?viewmonth=201707&viewday=15
 *   Tod Baudais for combining the above and cleaning up this example
 */

#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>

#include "gdcmReader.h"
#include "gdcmAttribute.h"
#include "gdcmDirectory.h"

//==============================================================================
//==============================================================================

#define TAG_MEDIA_STORAGE_SOP_CLASS_UID 0x0002,0x0002
#define TAG_DIRECTORY_RECORD_SEQUENCE 0x0004,0x1220
#define TAG_DIRECTORY_RECORD_TYPE 0x0004,0x1430
#define TAG_PATIENTS_NAME 0x0010,0x0010
#define TAG_PATIENT_ID 0x0010,0x0020
#define TAG_STUDY_DATE 0x0008,0x0020
#define TAG_STUDY_DESCRIPTION 0x0008,0x1030
#define TAG_MODALITY 0x0008,0x0060
#define TAG_SERIES_DESCRIPTION 0x0008,0x103E
#define TAG_REFERENCED_FILE_ID 0x0004,0x1500
#define TAG_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY_OFFSET 0x0004,0x1420
#define TAG_NEXT_DIRECTORY_RECORD_OFFSET 0x0004,0x1400

//==============================================================================
// Some handy utility functions
//==============================================================================

std::string left_trim(const std::string &s) {
    std::string ss(s);
    ss.erase(ss.begin(), std::find_if(ss.begin(), ss.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return ss;
}

std::string right_trim(const std::string &s) {
    std::string ss(s);
    ss.erase(std::find_if(ss.rbegin(), ss.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), ss.end());
    return ss;
}

std::string trim(const std::string &s) {
    return left_trim(right_trim(s));
}

//==============================================================================
// This code could be put in a header file somewhere
//==============================================================================

class DICOMDIRReader {
    public:
                                DICOMDIRReader      (void) {}
                                DICOMDIRReader      (const DICOMDIRReader &rhs) = delete;
                                DICOMDIRReader      (DICOMDIRReader &&rhs) = delete;
        DICOMDIRReader &        operator =          (const DICOMDIRReader &rhs) = delete;
        DICOMDIRReader &        operator =          (DICOMDIRReader &&rhs) = delete;
        virtual                 ~DICOMDIRReader     (void) {}

    public:
        struct Common {
            int64_t child_offset;
            int64_t sibling_offset;
        };

        struct Image: public Common {
            std::string path;
        };

        struct Series: public Common {
            std::string modality;
            std::string description;

            std::vector<std::shared_ptr<Image>> children;
        };

        struct Study: public Common {
            std::string date;
            std::string description;

            std::vector<std::shared_ptr<Series>> children;
        };

        struct Patient: public Common {
            std::string name;
            std::string id;

            std::vector<std::shared_ptr<Study>> children;
        };

        struct Other: public Common {
        };

        /// Load DICOMDIR
        const std::vector<std::shared_ptr<Patient>>&    load        (const std::string &path);

        /// Return the results of the load
        const std::vector<std::shared_ptr<Patient>>&    patients    (void)  {   return _patients;   }

    private:

        template <class T>
        std::string     get_string              (const T &ds, const gdcm::Tag &tag)
        {
            std::stringstream strm;
            if (ds.FindDataElement(tag)) {
                auto &de = ds.GetDataElement(tag);
                if (!de.IsEmpty() && !de.IsUndefinedLength())
                    de.GetValue().Print(strm);
            }
            return trim(strm.str());
        }

        template <class P, class C, class O>
        void            reassemble_hierarchy    (P &parent_offsets, C &child_offsets, O &other_offsets)
        {
            for (auto &parent : parent_offsets) {
                int64_t sibling_offset;
                auto c = child_offsets[parent.second->child_offset];
                if (!c) {
                    auto o = other_offsets[parent.second->child_offset];
                    if (!o) {
                        continue;
                    } else {
                        sibling_offset = o->sibling_offset;
                    }
                } else {
                    parent.second->children.push_back(c);
                    sibling_offset = c->sibling_offset;
                }

                // Get all siblings
                while (sibling_offset) {
                    c = child_offsets[sibling_offset];
                    if (!c) {
                        auto o = other_offsets[sibling_offset];
                        if (!o) {
                            break;
                        } else {
                            sibling_offset = o->sibling_offset;
                        }
                    } else {
                        parent.second->children.push_back(c);
                        sibling_offset = c->sibling_offset;
                    }
                }
            }
        }

        std::vector<std::shared_ptr<Patient>> _patients;
};

//==============================================================================
// This code could be put in an implementation file somewhere
//==============================================================================

const std::vector<std::shared_ptr<DICOMDIRReader::Patient>>& DICOMDIRReader::load (const std::string &path)
{
    _patients.clear();

    //
    // Read the dataset from the DICOMDIR file
    //

    gdcm::Reader reader;
    reader.SetFileName(path.c_str());
    if(!reader.Read()) {
        throw std::runtime_error("Unable to read file");
    }

    // Retrieve information from file
    auto &file = reader.GetFile();
    auto &data_set = file.GetDataSet();
    auto &file_meta_information = file.GetHeader();

    // Retrieve and check the Media Storage class from file
    gdcm::MediaStorage media_storage;
    media_storage.SetFromFile(file);
    if(media_storage != gdcm::MediaStorage::MediaStorageDirectoryStorage) {
        throw std::runtime_error("This file is not a DICOMDIR");
    }

    auto media_storage_sop_class_uid = get_string(file_meta_information, gdcm::Tag(TAG_MEDIA_STORAGE_SOP_CLASS_UID));

    // Make sure we have a DICOMDIR file
    if (media_storage_sop_class_uid != "1.2.840.10008.1.3.10") {
        throw std::runtime_error("This file is not a DICOMDIR");
    }

    //
    // Offset to first item courtesy of Mihail Isakov
    //

    gdcm::VL first_item_offset = 0;
    auto it = data_set.Begin();
    for(; it != data_set.End() && it->GetTag() != gdcm::Tag(TAG_DIRECTORY_RECORD_SEQUENCE); ++it) {
        first_item_offset += it->GetLength<gdcm::ExplicitDataElement>();
    }
    // Tag (4 bytes)
    first_item_offset += it->GetTag().GetLength();
    // VR field
    first_item_offset += it->GetVR().GetLength();
    // VL field
    // For Explicit VR: adventitiously VL field lenght = VR field lenght,
    // for SQ 4 bytes:
    // http://dicom.nema.org/medical/dicom/current/output/html/part05.html#table_7.1-1
    first_item_offset += it->GetVR().GetLength();

    //
    // Iterate all data elements
    //

    // For each item in data set
    for(auto data_element : data_set.GetDES()) {

        // Only look at Directory sequence
        if (data_element.GetTag() != gdcm::Tag(TAG_DIRECTORY_RECORD_SEQUENCE))
            continue;

        auto item_sequence = data_element.GetValueAsSQ();
        auto num_items = item_sequence->GetNumberOfItems();

        //
        // Compute an offset table
        //

        // Start calculation of offset to each item courtesy of Mihail Isakov
        std::vector<int64_t> item_offsets(num_items+1);
        item_offsets[0] = file_meta_information.GetFullLength() + static_cast<int64_t>(first_item_offset);

        //
        // Extract out all of the items
        //

        std::unordered_map<int64_t, std::shared_ptr<Patient>> patient_offsets;
        std::unordered_map<int64_t, std::shared_ptr<Study>> study_offsets;
        std::unordered_map<int64_t, std::shared_ptr<Series>> series_offsets;
        std::unordered_map<int64_t, std::shared_ptr<Image>> image_offsets;
        std::unordered_map<int64_t, std::shared_ptr<Other>> other_offsets;

        for (uint32_t item_index = 1; item_index <= num_items; ++item_index) {
            auto &item = item_sequence->GetItem(item_index);

            // Add offset for item to offset table
            item_offsets[item_index] = item_offsets[item_index-1] + item.GetLength<gdcm::ExplicitDataElement>();

            // Child offset
            gdcm::Attribute<TAG_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY_OFFSET> child_offset;
            child_offset.SetFromDataElement(item.GetDataElement(gdcm::Tag (TAG_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY_OFFSET)));

            // Sibling offset
            gdcm::Attribute<TAG_NEXT_DIRECTORY_RECORD_OFFSET> sibling_offset;
            sibling_offset.SetFromDataElement(item.GetDataElement(gdcm::Tag (TAG_NEXT_DIRECTORY_RECORD_OFFSET)));

            // Record Type
            auto record_type = trim(get_string(item, gdcm::Tag (TAG_DIRECTORY_RECORD_TYPE)));

            // std::cout << "record_type " << record_type << " at " << item_offsets[item_index-1] << std::endl;
            // std::cout << " child_offset " << child_offset.GetValue() << std::endl;
            // std::cout << " sibling_offset " << sibling_offset.GetValue() << std::endl;

            // Extract patient information
            if (record_type == "PATIENT") {
                auto patient = std::make_shared<Patient>();
                patient->name = get_string(item, gdcm::Tag (TAG_PATIENTS_NAME));
                patient->id = get_string(item, gdcm::Tag (TAG_PATIENT_ID));

                patient->child_offset = child_offset.GetValue();
                patient->sibling_offset = sibling_offset.GetValue();
                patient_offsets[item_offsets[item_index-1]] = patient;

            // Extract study information
            } else if (record_type == "STUDY") {
                auto study = std::make_shared<Study>();
                study->date = get_string(item, gdcm::Tag (TAG_STUDY_DATE));
                study->description = get_string(item, gdcm::Tag (TAG_STUDY_DESCRIPTION));

                study->child_offset = child_offset.GetValue();
                study->sibling_offset = sibling_offset.GetValue();
                study_offsets[item_offsets[item_index-1]] = study;

            // Extract series information
            } else if (record_type == "SERIES") {
                auto series = std::make_shared<Series>();
                series->modality = get_string(item, gdcm::Tag (TAG_MODALITY));
                series->description = get_string(item, gdcm::Tag (TAG_SERIES_DESCRIPTION));

                series->child_offset = child_offset.GetValue();
                series->sibling_offset = sibling_offset.GetValue();
                series_offsets[item_offsets[item_index-1]] = series;

            // Extract image information
            } else if (record_type == "IMAGE") {
                auto image = std::make_shared<Image>();
                image->path = get_string(item, gdcm::Tag (TAG_REFERENCED_FILE_ID));

                image->child_offset = child_offset.GetValue();
                image->sibling_offset = sibling_offset.GetValue();
                image_offsets[item_offsets[item_index-1]] = image;
            } else {
                auto other = std::make_shared<Other>();

                other->child_offset = child_offset.GetValue();
                other->sibling_offset = sibling_offset.GetValue();
                other_offsets[item_offsets[item_index-1]] = other;
            }
        }

        // Check validity
        if (patient_offsets.size() == 0)
            throw std::runtime_error("Unable to find patient record");

        reassemble_hierarchy(series_offsets, image_offsets, other_offsets);
        reassemble_hierarchy(study_offsets, series_offsets, other_offsets);
        reassemble_hierarchy(patient_offsets, study_offsets, other_offsets);

        // Set the new root
        for (auto &patient : patient_offsets) {
            _patients.push_back(patient.second);
        }
    }

    return _patients;
}

//==============================================================================
// Quick test
//==============================================================================

int main(int argc, const char *argv[]) {
    DICOMDIRReader reader;

    try {
        if (argc != 2)
            throw std::runtime_error("Wrong number of arguments");

        auto &patients = reader.load(argv[1]);

        for (auto &patient : patients) {

            std::cout << "PATIENT" << std::endl;
            std::cout << "NAME: " << patient->name << std::endl;
            std::cout << "ID: " << patient->id << std::endl;

            int x = 0;
            for (auto &study : patient->children) {
                std::cout << "    STUDY" << std::endl;
                std::cout << "    DESCRIPTION: " << study->description << std::endl;
                std::cout << "    DATE: " << study->date << std::endl;

                for (auto &series : study->children) {
                    x+=1;
                    std::cout << "        SERIES " << x << std::endl;
                    std::cout << "        DESCRIPTION: " << series->description << std::endl;
                    std::cout << "        MODALITY: " << series->modality << std::endl;

                    for (auto &image : series->children) {
                        std::cout << "            IMAGE PATH: " << image->path << std::endl;
                    }
                }
            }
        }
    }
    catch (...) {
        // TODO handle this
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
