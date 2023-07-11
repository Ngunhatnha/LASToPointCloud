#include <math.h>
#include "LasToLas.hpp"


int target,source;
std::string source_geoId;
bool isDestFile = false; //check if write info of the destination epsg
bool isDest = false; //Check if transform to destination epsg
bool change_geoId = false; //Check if change geoId 
GeoProjectionConverter geoprojectionconverter2;
bool isCrop = false; //Check if crop 
bool cropBefore = false;  //crop before or after the transform

const unsigned char convert_point_type_from_to[11][11] =
{
  {  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  1,  0,  1,  1,  1,  1,  1,  1 },
  {  0,  0,  1,  0,  1,  0,  1,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1 },
  {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 },
};

LasToLas::LasToLas()
{
    
}
LasToLas::~LasToLas()
{

}
void LasToLas::usage(bool error, bool wait) const
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "las2las -i *.las -utm 13N\n");
    fprintf(stderr, "las2las -i *.laz -first_only -olaz\n");
    fprintf(stderr, "las2las -i *.las -drop_return 4 5 -olaz\n");
    fprintf(stderr, "las2las -latlong -target_utm 12T -i in.las -o out.las\n");
    fprintf(stderr, "las2las -i in.laz -target_epsg 2972 -o out.laz\n");
    fprintf(stderr, "las2las -set_point_type 0 -lof file_list.txt -merged -o out.las\n");
    fprintf(stderr, "las2las -remove_vlr 2 -scale_rgb_up -i in.las -o out.las\n");
    fprintf(stderr, "las2las -i in.las -keep_xy 630000 4834500 630500 4835000 -keep_z 10 100 -o out.las\n");
    fprintf(stderr, "las2las -i in.txt -iparse xyzit -keep_circle 630200 4834750 100 -oparse xyzit -o out.txt\n");
    fprintf(stderr, "las2las -i in.las -remove_padding -keep_scan_angle -15 15 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -rescale 0.01 0.01 0.01 -reoffset 0 300000 0 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -set_version 1.2 -keep_gpstime 46.5 47.5 -o out.las\n");
    fprintf(stderr, "las2las -i in.las -drop_intensity_below 10 -olaz -stdout > out.laz\n");
    fprintf(stderr, "las2las -i in.las -last_only -drop_gpstime_below 46.75 -otxt -oparse xyzt -stdout > out.txt\n");
    fprintf(stderr, "las2las -i in.las -remove_all_vlrs -keep_class 2 3 4 -olas -stdout > out.las\n");
    fprintf(stderr, "las2las -h\n");
    if (wait)
    {
        fprintf(stderr, "<press ENTER>\n");
        getc(stdin);
    }
    exit(error);
}
void LasToLas::byebye(bool error, bool wait) const
{
    if (wait)
    {
        fprintf(stderr, "<press ENTER>\n");
        getc(stdin);
    }
    exit(error);
}
double LasToLas::taketime() const
{
    return (double)(clock()) / CLOCKS_PER_SEC;
}
bool LasToLas::save_vlrs_to_file(const LASheader* header) const
{
    U32 i;
    FILE* file = fopen("vlrs.vlr", "wb");
    if (file == 0)
    {
        return false;
    }
    ByteStreamOut* out;
    if (IS_LITTLE_ENDIAN())
        out = new ByteStreamOutFileLE(file);
    else
        out = new ByteStreamOutFileBE(file);
    // write number of VLRs
    if (!out->put32bitsLE((U8*)&(header->number_of_variable_length_records)))
    {
        fprintf(stderr, "ERROR: writing header->number_of_variable_length_records\n");
        return false;
    }
    // loop over VLRs
    for (i = 0; i < header->number_of_variable_length_records; i++)
    {
        if (!out->put16bitsLE((U8*)&(header->vlrs[i].reserved)))
        {
            fprintf(stderr, "ERROR: writing header->vlrs[%d].reserved\n", i);
            return false;
        }
        if (!out->putBytes((U8*)header->vlrs[i].user_id, 16))
        {
            fprintf(stderr, "ERROR: writing header->vlrs[%d].user_id\n", i);
            return false;
        }
        if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_id)))
        {
            fprintf(stderr, "ERROR: writing header->vlrs[%d].record_id\n", i);
            return false;
        }
        if (!out->put16bitsLE((U8*)&(header->vlrs[i].record_length_after_header)))
        {
            fprintf(stderr, "ERROR: writing header->vlrs[%d].record_length_after_header\n", i);
            return false;
        }
        if (!out->putBytes((U8*)header->vlrs[i].description, 32))
        {
            fprintf(stderr, "ERROR: writing header->vlrs[%d].description\n", i);
            return false;
        }

        // write the data following the header of the variable length record

        if (header->vlrs[i].record_length_after_header)
        {
            if (header->vlrs[i].data)
            {
                if (!out->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
                {
                    fprintf(stderr, "ERROR: writing %d bytes of data from header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
                    return false;
                }
            }
            else
            {
                fprintf(stderr, "ERROR: there should be %d bytes of data in header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
                return false;
            }
        }
    }
    delete out;
    fclose(file);
    return true;
}

bool LasToLas::tranform(const Vec3d& vSource, Vec3d& vDest, const Tranform& mToTarget, const Tranform& mToWGS84, const std::string& GeoId , const GeoidPROJ* pGeoidPROJ) const
{
    double xECEF, yECEF, zECEF, dLat, dLon, dHeight;
    mToWGS84.Convert(vSource.getY(), vSource.getX(), dLat, dLon);
    double dElev = vSource.getZ();
    if (GeoId == "") {
        dElev = vSource.getZ();
    }
    else if (pGeoidPROJ->getHeight(false, dLat, dLon, GeoId,dHeight))
        dElev += dHeight;
    if (mToTarget.Convert(dLat, dLon, dElev, xECEF, yECEF, zECEF))
    {
        vDest = Vec3d(xECEF, yECEF, zECEF);
        return true;
    }
    return false;
}

bool LasToLas::load_vlrs_from_file(LASheader* header) const
{
    U32 i;
    FILE* file = fopen("vlrs.vlr", "rb");
    if (file == 0)
    {
        return false;
    }
    ByteStreamIn* in;
    if (IS_LITTLE_ENDIAN())
        in = new ByteStreamInFileLE(file);
    else
        in = new ByteStreamInFileBE(file);
    // read number of VLRs
    U32 number_of_variable_length_records;
    try { in->get32bitsLE((U8*)&number_of_variable_length_records); }
    catch (...)
    {
        fprintf(stderr, "ERROR: reading number_of_variable_length_records\n");
        return false;
    }
    // loop over VLRs
    LASvlr vlr;
    for (i = 0; i < number_of_variable_length_records; i++)
    {
        try { in->get16bitsLE((U8*)&(vlr.reserved)); }
        catch (...)
        {
            fprintf(stderr, "ERROR: reading vlr.reserved\n");
            return false;
        }
        try { in->getBytes((U8*)vlr.user_id, 16); }
        catch (...)
        {
            fprintf(stderr, "ERROR: reading vlr.user_id\n");
            return false;
        }
        try { in->get16bitsLE((U8*)&(vlr.record_id)); }
        catch (...)
        {
            fprintf(stderr, "ERROR: reading vlr.record_id\n");
            return false;
        }
        try { in->get16bitsLE((U8*)&(vlr.record_length_after_header)); }
        catch (...)
        {
            fprintf(stderr, "ERROR: reading vlr.record_length_after_header\n");
            return false;
        }
        try { in->getBytes((U8*)vlr.description, 32); }
        catch (...)
        {
            fprintf(stderr, "ERROR: reading vlr.description\n");
            return false;
        }

        // write the data following the header of the variable length record

        if (vlr.record_length_after_header)
        {
            vlr.data = new U8[vlr.record_length_after_header];
            try { in->getBytes((U8*)vlr.data, vlr.record_length_after_header); }
            catch (...)
            {
                fprintf(stderr, "ERROR: reading %d bytes into vlr.data\n", vlr.record_length_after_header);
                return false;
            }
        }
        else
        {
            vlr.data = 0;
        }
        header->add_vlr(vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.data, TRUE, vlr.description);
    }
    delete in;
    fclose(file);
    return true;
}
int LasToLas::lidardouble2string(char* string, double value) const
{
    int len;
    len = sprintf(string, "%.15f", value) - 1;
    while (string[len] == '0') len--;
    if (string[len] != '.') len++;
    string[len] = '\0';
    return len;
}
I32 LasToLas::lidardouble2string(char* string, double value, double precision) const
{
    if (precision == 0.1)
        sprintf(string, "%.1f", value);
    else if (precision == 0.01)
        sprintf(string, "%.2f", value);
    else if (precision == 0.001 || precision == 0.002 || precision == 0.005 || precision == 0.025)
        sprintf(string, "%.3f", value);
    else if (precision == 0.0001 || precision == 0.0002 || precision == 0.0005 || precision == 0.0025)
        sprintf(string, "%.4f", value);
    else if (precision == 0.00001 || precision == 0.00002 || precision == 0.00005 || precision == 0.00025)
        sprintf(string, "%.5f", value);
    else if (precision == 0.000001)
        sprintf(string, "%.6f", value);
    else if (precision == 0.0000001)
        sprintf(string, "%.7f", value);
    else if (precision == 0.00000001)
        sprintf(string, "%.8f", value);
    else if (precision == 0.5)
        sprintf(string, "%.1f", value);
    else if (precision == 0.25)
        sprintf(string, "%.2f", value);
    else if (precision == 0.125)
        sprintf(string, "%.3f", value);
    else
        return lidardouble2string(string, value);
    return (I32)strlen(string) - 1;
}
bool LasToLas::valid_resolution(F64 coordinate, F64 offset, F64 scale_factor) const
{
    F64 coordinate_without_offset = coordinate - offset;
    F64 fixed_precision_multiplier = coordinate_without_offset / scale_factor;
    I64 quantized_fixed_precision_multiplier = I64_QUANTIZE(fixed_precision_multiplier);
    if ((fabs(fixed_precision_multiplier - quantized_fixed_precision_multiplier)) < 0.001)
    {
        return true;
    }
    return false;
}
int LasToLas::getInfor(LASreader* lasreader,GeoProjectionConverter* pGeoProjectionConverter, const char* fileName,unsigned int& nEPSG,std::string& strCS) const
{
    nEPSG = 0;
    strCS = "";
    CHAR printstring[4096];
    FILE* file_out = fopen(fileName, "wt");
    if (file_out == nullptr || lasreader==nullptr)
        return -1;
    LASheader* lasheader = &(lasreader->header);
    bool no_header = (lasheader == nullptr);
    bool no_warnings = true;
    bool no_variable_header = false;
    U32 horizontal_units = 0;
    //GeoProjectionConverter geoprojectionconverter;

    if (file_out && !no_header)
    {            
    fprintf(file_out, "reporting all LAS header entries:\012");
    fprintf(file_out, "  file signature:             '%.4s'\012", lasheader->file_signature);
    fprintf(file_out, "  file source ID:             %d\012", lasheader->file_source_ID);
    fprintf(file_out, "  global_encoding:            %d\012", lasheader->global_encoding);
    fprintf(file_out, "  project ID GUID data 1-4:   %08X-%04X-%04X-%04X-%04X%08X\012", lasheader->project_ID_GUID_data_1, lasheader->project_ID_GUID_data_2, lasheader->project_ID_GUID_data_3, *((U16*)(lasheader->project_ID_GUID_data_4)), *((U16*)(lasheader->project_ID_GUID_data_4 + 2)), *((U32*)(lasheader->project_ID_GUID_data_4 + 4)));
    fprintf(file_out, "  version major.minor:        %d.%d\012", lasheader->version_major, lasheader->version_minor);
    fprintf(file_out, "  system identifier:          '%.32s'\012", lasheader->system_identifier);
    fprintf(file_out, "  generating software:        '%.32s'\012", lasheader->generating_software);
    fprintf(file_out, "  file creation day/year:     %d/%d\012", lasheader->file_creation_day, lasheader->file_creation_year);
    fprintf(file_out, "  header size:                %d\012", lasheader->header_size);
    fprintf(file_out, "  offset to point data:       %u\012", lasheader->offset_to_point_data);
    fprintf(file_out, "  number var. length records: %u\012", lasheader->number_of_variable_length_records);
    fprintf(file_out, "  point data format:          %d\012", lasheader->point_data_format);
    fprintf(file_out, "  point data record length:   %d\012", lasheader->point_data_record_length);
    fprintf(file_out, "  number of point records:    %u\012", lasheader->number_of_point_records);
    fprintf(file_out, "  number of points by return: %u %u %u %u %u\012", lasheader->number_of_points_by_return[0], lasheader->number_of_points_by_return[1], lasheader->number_of_points_by_return[2], lasheader->number_of_points_by_return[3], lasheader->number_of_points_by_return[4]);
    fprintf(file_out, "  scale factor x y z:         "); lidardouble2string(printstring, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
    fprintf(file_out, "  offset x y z:               "); lidardouble2string(printstring, lasheader->x_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->y_offset); fprintf(file_out, "%s ", printstring);  lidardouble2string(printstring, lasheader->z_offset); fprintf(file_out, "%s\012", printstring);
    fprintf(file_out, "  min x y z:                  "); lidardouble2string(printstring, lasheader->min_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->min_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
    fprintf(file_out, "  max x y z:                  "); lidardouble2string(printstring, lasheader->max_x, lasheader->x_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_y, lasheader->y_scale_factor); fprintf(file_out, "%s ", printstring); lidardouble2string(printstring, lasheader->max_z, lasheader->z_scale_factor); fprintf(file_out, "%s\012", printstring);
    if (!no_warnings && !valid_resolution(lasheader->min_x, lasheader->x_offset, lasheader->x_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of min_x not compatible with x_offset and x_scale_factor: "); lidardouble2string(printstring, lasheader->min_x); fprintf(file_out, "%s\n", printstring);
    }
    if (!no_warnings && !valid_resolution(lasheader->min_y, lasheader->y_offset, lasheader->y_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of min_y not compatible with y_offset and y_scale_factor: "); lidardouble2string(printstring, lasheader->min_y); fprintf(file_out, "%s\n", printstring);
    }
    if (!no_warnings && !valid_resolution(lasheader->min_z, lasheader->z_offset, lasheader->z_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of min_z not compatible with z_offset and z_scale_factor: "); lidardouble2string(printstring, lasheader->min_z); fprintf(file_out, "%s\n", printstring);
    }
    if (!no_warnings && !valid_resolution(lasheader->max_x, lasheader->x_offset, lasheader->x_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of max_x not compatible with x_offset and x_scale_factor: "); lidardouble2string(printstring, lasheader->max_x); fprintf(file_out, "%s\n", printstring);
    }
    if (!no_warnings && !valid_resolution(lasheader->max_y, lasheader->y_offset, lasheader->y_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of max_y not compatible with y_offset and y_scale_factor: "); lidardouble2string(printstring, lasheader->max_y); fprintf(file_out, "%s\n", printstring);
    }
    if (!no_warnings && !valid_resolution(lasheader->max_z, lasheader->z_offset, lasheader->z_scale_factor))
    {
        fprintf(file_out, "WARNING: stored resolution of max_z not compatible with z_offset and z_scale_factor: "); lidardouble2string(printstring, lasheader->max_z); fprintf(file_out, "%s\n", printstring);
    }
    if ((lasheader->version_major == 1) && (lasheader->version_minor >= 3))
    {
#ifdef _WIN32
        fprintf(file_out, "  start of waveform data packet record: %I64d\012", lasheader->start_of_waveform_data_packet_record);
#else
        fprintf(file_out, "  start of waveform data packet record: %lld\012", lasheader->start_of_waveform_data_packet_record);
#endif
    }
    if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4))
    {
#ifdef _WIN32
        fprintf(file_out, "  start of first extended variable length record: %I64d\012", lasheader->start_of_first_extended_variable_length_record);
#else
        fprintf(file_out, "  start of first extended variable length record: %lld\012", lasheader->start_of_first_extended_variable_length_record);
#endif
        fprintf(file_out, "  number of extended_variable length records: %d\012", lasheader->number_of_extended_variable_length_records);
#ifdef _WIN32
        fprintf(file_out, "  extended number of point records: %I64d\012", lasheader->extended_number_of_point_records);
        fprintf(file_out, "  extended number of points by return: %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d %I64d\012", lasheader->extended_number_of_points_by_return[0], lasheader->extended_number_of_points_by_return[1], lasheader->extended_number_of_points_by_return[2], lasheader->extended_number_of_points_by_return[3], lasheader->extended_number_of_points_by_return[4], lasheader->extended_number_of_points_by_return[5], lasheader->extended_number_of_points_by_return[6], lasheader->extended_number_of_points_by_return[7], lasheader->extended_number_of_points_by_return[8], lasheader->extended_number_of_points_by_return[9], lasheader->extended_number_of_points_by_return[10], lasheader->extended_number_of_points_by_return[11], lasheader->extended_number_of_points_by_return[12], lasheader->extended_number_of_points_by_return[13], lasheader->extended_number_of_points_by_return[14]);
#else
        fprintf(file_out, "  extended number of point records: %lld\012", lasheader->extended_number_of_point_records);
        fprintf(file_out, "  extended number of points by return: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\012", lasheader->extended_number_of_points_by_return[0], lasheader->extended_number_of_points_by_return[1], lasheader->extended_number_of_points_by_return[2], lasheader->extended_number_of_points_by_return[3], lasheader->extended_number_of_points_by_return[4], lasheader->extended_number_of_points_by_return[5], lasheader->extended_number_of_points_by_return[6], lasheader->extended_number_of_points_by_return[7], lasheader->extended_number_of_points_by_return[8], lasheader->extended_number_of_points_by_return[9], lasheader->extended_number_of_points_by_return[10], lasheader->extended_number_of_points_by_return[11], lasheader->extended_number_of_points_by_return[12], lasheader->extended_number_of_points_by_return[13], lasheader->extended_number_of_points_by_return[14]);
#endif
    }
    if (lasheader->user_data_in_header_size) fprintf(file_out, "the header contains %u user-defined bytes\012", lasheader->user_data_in_header_size);
}

// maybe print variable header

if (file_out && !no_variable_header)
{
    for (int i = 0; i < (int)lasheader->number_of_variable_length_records; i++)
    {
        fprintf(file_out, "variable length header record %d of %d:\012", i + 1, (int)lasheader->number_of_variable_length_records);
        fprintf(file_out, "  reserved             %d\012", lasreader->header.vlrs[i].reserved);
        fprintf(file_out, "  user ID              '%.16s'\012", lasreader->header.vlrs[i].user_id);
        fprintf(file_out, "  record ID            %d\012", lasreader->header.vlrs[i].record_id);
        fprintf(file_out, "  length after header  %d\012", lasreader->header.vlrs[i].record_length_after_header);
        fprintf(file_out, "  description          '%.32s'\012", lasreader->header.vlrs[i].description);

        // special handling for known variable header tags

        if ((strcmp(lasheader->vlrs[i].user_id, "LASF_Projection") == 0) && (lasheader->vlrs[i].data != 0))
        {
            if (lasheader->vlrs[i].record_id == 34735) // GeoKeyDirectoryTag
            {
                fprintf(file_out, "    GeoKeyDirectoryTag version %d.%d.%d number of keys %d\012", lasheader->vlr_geo_keys->key_directory_version, lasheader->vlr_geo_keys->key_revision, lasheader->vlr_geo_keys->minor_revision, lasheader->vlr_geo_keys->number_of_keys);
                for (int j = 0; j < lasheader->vlr_geo_keys->number_of_keys; j++)
                {
                    if (file_out)
                    {
                        fprintf(file_out, "      key %d tiff_tag_location %d count %d value_offset %d - ", lasheader->vlr_geo_key_entries[j].key_id, lasheader->vlr_geo_key_entries[j].tiff_tag_location, lasheader->vlr_geo_key_entries[j].count, lasheader->vlr_geo_key_entries[j].value_offset);
                        switch (lasreader->header.vlr_geo_key_entries[j].key_id)
                        {
                        case 1024: // GTModelTypeGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 1: // ModelTypeProjected   
                                fprintf(file_out, "GTModelTypeGeoKey: ModelTypeProjected\012");
                                break;
                            case 2:
                                fprintf(file_out, "GTModelTypeGeoKey: ModelTypeGeographic\012");
                                break;
                            case 3:
                                fprintf(file_out, "GTModelTypeGeoKey: ModelTypeGeocentric\012");
                                break;
                            case 0: // ModelTypeUndefined   
                                fprintf(file_out, "GTModelTypeGeoKey: ModelTypeUndefined\012");
                                break;
                            default:
                                fprintf(file_out, "GTModelTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 1025: // GTRasterTypeGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 1: // RasterPixelIsArea   
                                fprintf(file_out, "GTRasterTypeGeoKey: RasterPixelIsArea\012");
                                break;
                            case 2: // RasterPixelIsPoint
                                fprintf(file_out, "GTRasterTypeGeoKey: RasterPixelIsPoint\012");
                                break;
                            default:
                                fprintf(file_out, "GTRasterTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 1026: // GTCitationGeoKey
                            if (lasreader->header.vlr_geo_ascii_params)
                            {
                                char dummy[256];
                                strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                                dummy[lasreader->header.vlr_geo_key_entries[j].count - 1] = '\0';
                                fprintf(file_out, "GTCitationGeoKey: %s\012", dummy);
                            }
                            break;
                        case 2048: // GeographicTypeGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 32767: // user-defined
                                fprintf(file_out, "GeographicTypeGeoKey: user-defined\012");
                                break;
                            case 4001: // GCSE_Airy1830
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Airy1830\012");
                                break;
                            case 4002: // GCSE_AiryModified1849 
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_AiryModified1849\012");
                                break;
                            case 4003: // GCSE_AustralianNationalSpheroid
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_AustralianNationalSpheroid\012");
                                break;
                            case 4004: // GCSE_Bessel1841
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Bessel1841\012");
                                break;
                            case 4005: // GCSE_Bessel1841Modified
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Bessel1841Modified\012");
                                break;
                            case 4006: // GCSE_BesselNamibia
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_BesselNamibia\012");
                                break;
                            case 4008: // GCSE_Clarke1866
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1866\012");
                                break;
                            case 4009: // GCSE_Clarke1866Michigan
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1866Michigan\012");
                                break;
                            case 4010: // GCSE_Clarke1880_Benoit
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_Benoit\012");
                                break;
                            case 4011: // GCSE_Clarke1880_IGN
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_IGN\012");
                                break;
                            case 4012: // GCSE_Clarke1880_RGS
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_RGS\012");
                                break;
                            case 4013: // GCSE_Clarke1880_Arc
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_Arc\012");
                                break;
                            case 4014: // GCSE_Clarke1880_SGA1922
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880_SGA1922\012");
                                break;
                            case 4015: // GCSE_Everest1830_1937Adjustment
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1937Adjustment\012");
                                break;
                            case 4016: // GCSE_Everest1830_1967Definition
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1967Definition\012");
                                break;
                            case 4017: // GCSE_Everest1830_1975Definition
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830_1975Definition\012");
                                break;
                            case 4018: // GCSE_Everest1830Modified
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Everest1830Modified\012");
                                break;
                            case 4019: // GCSE_GRS1980
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_GRS1980\012");
                                break;
                            case 4020: // GCSE_Helmert1906
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Helmert1906\012");
                                break;
                            case 4022: // GCSE_International1924
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_International1924\012");
                                break;
                            case 4023: // GCSE_International1967
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_International1967\012");
                                break;
                            case 4024: // GCSE_Krassowsky1940
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Krassowsky1940\012");
                                break;
                            case 4030: // GCSE_WGS84
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_WGS84\012");
                                break;
                            case 4034: // GCSE_Clarke1880
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_Clarke1880\012");
                                break;
                            case 4140: // GCSE_NAD83_CSRS
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_NAD83_CSRS\012");
                                break;
                            case 4167: // GCSE_New_Zealand_Geodetic_Datum_2000
                                fprintf(file_out, "GeographicTypeGeoKey: GCSE_New_Zealand_Geodetic_Datum_2000\012");
                                break;
                            case 4267: // GCS_NAD27
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD27\012");
                                break;
                            case 4269: // GCS_NAD83
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83\012");
                                break;
                            case 4283: // GCS_GDA94
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_GDA94\012");
                                break;
                            case 4312: // GCS_MGI
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_MGI\012");
                                break;
                            case 4322: // GCS_WGS_72
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_WGS_72\012");
                                break;
                            case 4326: // GCS_WGS_84
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_WGS_84\012");
                                break;
                            case 4289: // GCS_Amersfoort
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_Amersfoort\012");
                                break;
                            case 4617: // GCS_NAD83_CSRS
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_CSRS\012");
                                break;
                            case 4619: // GCS_SWEREF99
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_SWEREF99\012");
                                break;
                            case 6318: // GCS_NAD83_2011
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_2011\012");
                                break;
                            case 6322: // GCS_NAD83_PA11
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_NAD83_PA11\012");
                                break;
                            case 7844: // GCS_GDA2020
                                fprintf(file_out, "GeographicTypeGeoKey: GCS_GDA2020\012");
                                break;
                            default:
                                fprintf(file_out, "GeographicTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            if (isDest) {
                                nEPSG = lasreader->header.vlr_geo_key_entries[j].value_offset;
                                strCS = std::string(pGeoProjectionConverter->get_gcs_name());
                            }
                            break;
                        case 2049: // GeogCitationGeoKey
                            if (lasreader->header.vlr_geo_ascii_params)
                            {
                                char dummy[256];
                                strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                                dummy[lasreader->header.vlr_geo_key_entries[j].count - 1] = '\0';
                                fprintf(file_out, "GeogCitationGeoKey: %s\012", dummy);
                            }
                            break;
                        case 2050: // GeogGeodeticDatumGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 32767: // user-defined
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: user-defined\012");
                                break;
                            case 6202: // Datum_Australian_Geodetic_Datum_1966
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Australian_Geodetic_Datum_1966\012");
                                break;
                            case 6203: // Datum_Australian_Geodetic_Datum_1984
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Australian_Geodetic_Datum_1984\012");
                                break;
                            case 6267: // Datum_North_American_Datum_1927
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_North_American_Datum_1927\012");
                                break;
                            case 6269: // Datum_North_American_Datum_1983
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_North_American_Datum_1983\012");
                                break;
                            case 6283: // Datum_Geocentric_Datum_of_Australia_1994
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Geocentric_Datum_of_Australia_1994\012");
                                break;
                            case 6322: // Datum_WGS72
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_WGS72\012");
                                break;
                            case 6326: // Datum_WGS84
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_WGS84\012");
                                break;
                            case 6140: // Datum_WGS84
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_NAD83_CSRS\012");
                                break;
                            case 6619: // Datum_SWEREF99
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_SWEREF99\012");
                                break;
                            case 6289: // Datum_Amersfoort
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_Amersfoort\012");
                                break;
                            case 6167: // Datum_NZGD2000
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: Datum_NZGD2000\012");
                                break;
                            case 6001: // DatumE_Airy1830
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Airy1830\012");
                                break;
                            case 6002: // DatumE_AiryModified1849
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_AiryModified1849\012");
                                break;
                            case 6003: // DatumE_AustralianNationalSpheroid
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_AustralianNationalSpheroid\012");
                                break;
                            case 6004: // DatumE_Bessel1841
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Bessel1841\012");
                                break;
                            case 6005: // DatumE_BesselModified
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_BesselModified\012");
                                break;
                            case 6006: // DatumE_BesselNamibia
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_BesselNamibia\012");
                                break;
                            case 6008: // DatumE_Clarke1866
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1866\012");
                                break;
                            case 6009: // DatumE_Clarke1866Michigan
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1866Michigan\012");
                                break;
                            case 6010: // DatumE_Clarke1880_Benoit
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_Benoit\012");
                                break;
                            case 6011: // DatumE_Clarke1880_IGN
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_IGN\012");
                                break;
                            case 6012: // DatumE_Clarke1880_RGS
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_RGS\012");
                                break;
                            case 6013: // DatumE_Clarke1880_Arc
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_Arc\012");
                                break;
                            case 6014: // DatumE_Clarke1880_SGA1922
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880_SGA1922\012");
                                break;
                            case 6015: // DatumE_Everest1830_1937Adjustment
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1937Adjustment\012");
                                break;
                            case 6016: // DatumE_Everest1830_1967Definition
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1967Definition\012");
                                break;
                            case 6017: // DatumE_Everest1830_1975Definition
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830_1975Definition\012");
                                break;
                            case 6018: // DatumE_Everest1830Modified
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Everest1830Modified\012");
                                break;
                            case 6019: // DatumE_GRS1980
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_GRS1980\012");
                                break;
                            case 6020: // DatumE_Helmert1906
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Helmert1906\012");
                                break;
                            case 6022: // DatumE_International1924
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_International1924\012");
                                break;
                            case 6023: // DatumE_International1967
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_International1967\012");
                                break;
                            case 6024: // DatumE_Krassowsky1940
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Krassowsky1940\012");
                                break;
                            case 6030: // DatumE_WGS84
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_WGS84\012");
                                break;
                            case 6034: // DatumE_Clarke1880
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: DatumE_Clarke1880\012");
                                break;
                            default:
                                fprintf(file_out, "GeogGeodeticDatumGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2051: // GeogPrimeMeridianGeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 32767: // user-defined
                                fprintf(file_out, "GeogPrimeMeridianGeoKey: user-defined\012");
                                break;
                            case 8901: // PM_Greenwich
                                fprintf(file_out, "GeogPrimeMeridianGeoKey: PM_Greenwich\012");
                                break;
                            case 8902: // PM_Lisbon
                                fprintf(file_out, "GeogPrimeMeridianGeoKey: PM_Lisbon\012");
                                break;
                            default:
                                fprintf(file_out, "GeogPrimeMeridianGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2052: // GeogLinearUnitsGeoKey 
                            horizontal_units = lasreader->header.vlr_geo_key_entries[j].value_offset;
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 9001: // Linear_Meter
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Meter\012");
                                break;
                            case 9002: // Linear_Foot
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot\012");
                                break;
                            case 9003: // Linear_Foot_US_Survey
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_US_Survey\012");
                                break;
                            case 9004: // Linear_Foot_Modified_American
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Modified_American\012");
                                break;
                            case 9005: // Linear_Foot_Clarke
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Clarke\012");
                                break;
                            case 9006: // Linear_Foot_Indian
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Foot_Indian\012");
                                break;
                            case 9007: // Linear_Link
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link\012");
                                break;
                            case 9008: // Linear_Link_Benoit
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link_Benoit\012");
                                break;
                            case 9009: // Linear_Link_Sears
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Link_Sears\012");
                                break;
                            case 9010: // Linear_Chain_Benoit
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Chain_Benoit\012");
                                break;
                            case 9011: // Linear_Chain_Sears
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Chain_Sears\012");
                                break;
                            case 9012: // Linear_Yard_Sears
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Yard_Sears\012");
                                break;
                            case 9013: // Linear_Yard_Indian
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Yard_Indian\012");
                                break;
                            case 9014: // Linear_Fathom
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Fathom\012");
                                break;
                            case 9015: // Linear_Mile_International_Nautical
                                fprintf(file_out, "GeogLinearUnitsGeoKey: Linear_Mile_International_Nautical\012");
                                break;
                            default:
                                fprintf(file_out, "GeogLinearUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2053: // GeogLinearUnitSizeGeoKey  
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogLinearUnitSizeGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2054: // GeogAngularUnitsGeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 9101: // Angular_Radian
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Radian\012");
                                break;
                            case 9102: // Angular_Degree
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Degree\012");
                                break;
                            case 9103: // Angular_Arc_Minute
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Arc_Minute\012");
                                break;
                            case 9104: // Angular_Arc_Second
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Arc_Second\012");
                                break;
                            case 9105: // Angular_Grad
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Grad\012");
                                break;
                            case 9106: // Angular_Gon
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_Gon\012");
                                break;
                            case 9107: // Angular_DMS
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_DMS\012");
                                break;
                            case 9108: // Angular_DMS_Hemisphere
                                fprintf(file_out, "GeogAngularUnitsGeoKey: Angular_DMS_Hemisphere\012");
                                break;
                            default:
                                fprintf(file_out, "GeogAngularUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2055: // GeogAngularUnitSizeGeoKey 
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogAngularUnitSizeGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2056: // GeogEllipsoidGeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 32767: // user-defined
                                fprintf(file_out, "GeogEllipsoidGeoKey: user-defined\012");
                                break;
                            case 7001: // Ellipse_Airy_1830
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Airy_1830\012");
                                break;
                            case 7002: // Ellipse_Airy_Modified_1849
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Airy_Modified_1849\012");
                                break;
                            case 7003: // Ellipse_Australian_National_Spheroid
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Australian_National_Spheroid\012");
                                break;
                            case 7004: // Ellipse_Bessel_1841
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_1841\012");
                                break;
                            case 7005: // Ellipse_Bessel_Modified
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_Modified\012");
                                break;
                            case 7006: // Ellipse_Bessel_Namibia
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Bessel_Namibia\012");
                                break;
                            case 7008: // Ellipse_Clarke_1866
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1866\012");
                                break;
                            case 7009: // Ellipse_Clarke_1866_Michigan
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1866_Michigan\012");
                                break;
                            case 7010: // Ellipse_Clarke1880_Benoit
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_Benoit\012");
                                break;
                            case 7011: // Ellipse_Clarke1880_IGN
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_IGN\012");
                                break;
                            case 7012: // Ellipse_Clarke1880_RGS
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_RGS\012");
                                break;
                            case 7013: // Ellipse_Clarke1880_Arc
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_Arc\012");
                                break;
                            case 7014: // Ellipse_Clarke1880_SGA1922
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke1880_SGA1922\012");
                                break;
                            case 7015: // Ellipse_Everest1830_1937Adjustment
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1937Adjustment\012");
                                break;
                            case 7016: // Ellipse_Everest1830_1967Definition
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1967Definition\012");
                                break;
                            case 7017: // Ellipse_Everest1830_1975Definition
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830_1975Definition\012");
                                break;
                            case 7018: // Ellipse_Everest1830Modified
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Everest1830Modified\012");
                                break;
                            case 7019: // Ellipse_GRS_1980
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_GRS_1980\012");
                                break;
                            case 7020: // Ellipse_Helmert1906
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Helmert1906\012");
                                break;
                            case 7022: // Ellipse_International1924
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_International1924\012");
                                break;
                            case 7023: // Ellipse_International1967
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_International1967\012");
                                break;
                            case 7024: // Ellipse_Krassowsky1940
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Krassowsky1940\012");
                                break;
                            case 7030: // Ellipse_WGS_84
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_WGS_84\012");
                                break;
                            case 7034: // Ellipse_Clarke_1880
                                fprintf(file_out, "GeogEllipsoidGeoKey: Ellipse_Clarke_1880\012");
                                break;
                            default:
                                fprintf(file_out, "GeogEllipsoidGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2057: // GeogSemiMajorAxisGeoKey 
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogSemiMajorAxisGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2058: // GeogSemiMinorAxisGeoKey 
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogSemiMinorAxisGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2059: // GeogInvFlatteningGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogInvFlatteningGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2060: // GeogAzimuthUnitsGeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 9101: // Angular_Radian
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Radian\012");
                                break;
                            case 9102: // Angular_Degree
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Degree\012");
                                break;
                            case 9103: // Angular_Arc_Minute
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Arc_Minute\012");
                                break;
                            case 9104: // Angular_Arc_Second
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Arc_Second\012");
                                break;
                            case 9105: // Angular_Grad
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Grad\012");
                                break;
                            case 9106: // Angular_Gon
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_Gon\012");
                                break;
                            case 9107: // Angular_DMS
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_DMS\012");
                                break;
                            case 9108: // Angular_DMS_Hemisphere
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: Angular_DMS_Hemisphere\012");
                                break;
                            default:
                                fprintf(file_out, "GeogAzimuthUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 2061: // GeogPrimeMeridianLongGeoKey  
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "GeogPrimeMeridianLongGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 2062: // GeogTOWGS84GeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].count)
                            {
                            case 3:
                                if (lasreader->header.vlr_geo_double_params)
                                {
                                    fprintf(file_out, "GeogTOWGS84GeoKey: TOWGS84[%.10g,%.10g,%.10g]\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 1], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 2]);
                                }
                                else
                                {
                                    fprintf(file_out, "GeogTOWGS84GeoKey: no vlr_geo_double_params. cannot look up the three parameters.\012");
                                }
                                break;
                            case 7:
                                if (lasreader->header.vlr_geo_double_params)
                                {
                                    fprintf(file_out, "GeogTOWGS84GeoKey: TOWGS84[%.10g,%.10g,%.10g,%.10g,%.10g,%.10g,%.10g]\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 1], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 2], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 3], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 4], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 5], lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset + 6]);
                                }
                                else
                                {
                                    fprintf(file_out, "GeogTOWGS84GeoKey: no vlr_geo_double_params. cannot look up the seven parameters.\012");
                                }
                                break;
                            default:
                                fprintf(file_out, "GeogTOWGS84GeoKey: look-up for type %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].count);
                            }
                            break;
                        case 3072: // ProjectedCSTypeGeoKey                        
                            if (!isDest || (isDest && isDestFile ) ) {
                                if (pGeoProjectionConverter->set_ProjectedCSTypeGeoKey(lasreader->header.vlr_geo_key_entries[j].value_offset, printstring))
                                {
                                nEPSG = lasreader->header.vlr_geo_key_entries[j].value_offset;
                                strCS = std::string(printstring);
                                horizontal_units = pGeoProjectionConverter->get_ProjLinearUnitsGeoKey();
                                fprintf(file_out, "ProjectedCSTypeGeoKey: %s\012", printstring);
                                break;
                                }
                                else
                                {
                                    fprintf(file_out, "ProjectedCSTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                                }
                            }
                            else {
                                isDestFile = true;
                                nEPSG = lasreader->header.vlr_geo_key_entries[j].value_offset;
                                strCS = std::string(pGeoProjectionConverter->get_projection_name(true));
                                horizontal_units = pGeoProjectionConverter->get_ProjLinearUnitsGeoKey();
                                fprintf(file_out, "ProjectedCSTypeGeoKey: %s\012", pGeoProjectionConverter->get_projection_name(true));
                                break;
                            }

                            /*
                                if (pGeoProjectionConverter->set_ProjectedCSTypeGeoKey(lasreader->header.vlr_geo_key_entries[j].value_offset, printstring))
                                {
                                    nEPSG = lasreader->header.vlr_geo_key_entries[j].value_offset;
                                    strCS = std::string(printstring);
                                    horizontal_units = pGeoProjectionConverter->get_ProjLinearUnitsGeoKey();
                                    fprintf(file_out, "ProjectedCSTypeGeoKey: %s\012", printstring);
                                    if(isDest) pGeoProjectionConverter->set_ProjectedCSTypeGeoKey(target, printstring);
                                    break;
                                }
                                else
                                {
                                    fprintf(file_out, "ProjectedCSTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                                }*/
                            break;
                        case 3073: // PCSCitationGeoKey
                            if (lasreader->header.vlr_geo_ascii_params)
                            {
                                char dummy[256];
                                strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                                dummy[lasreader->header.vlr_geo_key_entries[j].count - 1] = '\0';
                                fprintf(file_out, "PCSCitationGeoKey: %s\012", dummy);
                            }
                            break;
                        case 3074: // ProjectionGeoKey
                            if ((16001 <= lasreader->header.vlr_geo_key_entries[j].value_offset) && (lasreader->header.vlr_geo_key_entries[j].value_offset <= 16060))
                            {
                                fprintf(file_out, "ProjectionGeoKey: Proj_UTM_zone_%dN\012", lasreader->header.vlr_geo_key_entries[j].value_offset - 16000);
                            }
                            else if ((16101 <= lasreader->header.vlr_geo_key_entries[j].value_offset) && (lasreader->header.vlr_geo_key_entries[j].value_offset <= 16160))
                            {
                                fprintf(file_out, "ProjectionGeoKey: Proj_UTM_zone_%dS\012", lasreader->header.vlr_geo_key_entries[j].value_offset - 16100);
                            }
                            else
                            {
                                switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                                {
                                case 32767: // user-defined
                                    fprintf(file_out, "ProjectionGeoKey: user-defined\012");
                                    break;
                                case 10101: // Proj_Alabama_CS27_East
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS27_East\012");
                                    break;
                                case 10102: // Proj_Alabama_CS27_West
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS27_West\012");
                                    break;
                                case 10131: // Proj_Alabama_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS83_East\012");
                                    break;
                                case 10132: // Proj_Alabama_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alabama_CS83_West\012");
                                    break;
                                case 10201: // Proj_Arizona_Coordinate_System_east			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_east\012");
                                    break;
                                case 10202: // Proj_Arizona_Coordinate_System_Central		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_Central\012");
                                    break;
                                case 10203: // Proj_Arizona_Coordinate_System_west			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_Coordinate_System_west\012");
                                    break;
                                case 10231: // Proj_Arizona_CS83_east				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_east\012");
                                    break;
                                case 10232: // Proj_Arizona_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_Central\012");
                                    break;
                                case 10233: // Proj_Arizona_CS83_west				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arizona_CS83_west\012");
                                    break;
                                case 10301: // Proj_Arkansas_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS27_North\012");
                                    break;
                                case 10302: // Proj_Arkansas_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS27_South\012");
                                    break;
                                case 10331: // Proj_Arkansas_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS83_North\012");
                                    break;
                                case 10332: // Proj_Arkansas_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Arkansas_CS83_South\012");
                                    break;
                                case 10401: // Proj_California_CS27_I				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_I\012");
                                    break;
                                case 10402: // Proj_California_CS27_II				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_II\012");
                                    break;
                                case 10403: // Proj_California_CS27_III				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_III\012");
                                    break;
                                case 10404: // Proj_California_CS27_IV				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_IV\012");
                                    break;
                                case 10405: // Proj_California_CS27_V				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_V\012");
                                    break;
                                case 10406: // Proj_California_CS27_VI				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_VI\012");
                                    break;
                                case 10407: // Proj_California_CS27_VII				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS27_VII\012");
                                    break;
                                case 10431: // Proj_California_CS83_1				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_1\012");
                                    break;
                                case 10432: // Proj_California_CS83_2				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_2\012");
                                    break;
                                case 10433: // Proj_California_CS83_3				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_3\012");
                                    break;
                                case 10434: // Proj_California_CS83_4				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_4\012");
                                    break;
                                case 10435: // Proj_California_CS83_5				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_5\012");
                                    break;
                                case 10436: // Proj_California_CS83_6				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_California_CS83_6\012");
                                    break;
                                case 10501: // Proj_Colorado_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_North\012");
                                    break;
                                case 10502: // Proj_Colorado_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_Central\012");
                                    break;
                                case 10503: // Proj_Colorado_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS27_South\012");
                                    break;
                                case 10531: // Proj_Colorado_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_North\012");
                                    break;
                                case 10532: // Proj_Colorado_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_Central\012");
                                    break;
                                case 10533: // Proj_Colorado_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colorado_CS83_South\012");
                                    break;
                                case 10600: // Proj_Connecticut_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Connecticut_CS27\012");
                                    break;
                                case 10630: // Proj_Connecticut_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Connecticut_CS83\012");
                                    break;
                                case 10700: // Proj_Delaware_CS27					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Delaware_CS27\012");
                                    break;
                                case 10730: // Proj_Delaware_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Delaware_CS83\012");
                                    break;
                                case 10901: // Proj_Florida_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_East\012");
                                    break;
                                case 10902: // Proj_Florida_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_West\012");
                                    break;
                                case 10903: // Proj_Florida_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS27_North\012");
                                    break;
                                case 10931: // Proj_Florida_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_East\012");
                                    break;
                                case 10932: // Proj_Florida_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_West\012");
                                    break;
                                case 10933: // Proj_Florida_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Florida_CS83_North\012");
                                    break;
                                case 11001: // Proj_Georgia_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS27_East\012");
                                    break;
                                case 11002: // Proj_Georgia_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS27_West\012");
                                    break;
                                case 11031: // Proj_Georgia_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS83_East\012");
                                    break;
                                case 11032: // Proj_Georgia_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Georgia_CS83_West\012");
                                    break;
                                case 11101: // Proj_Idaho_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_East\012");
                                    break;
                                case 11102: // Proj_Idaho_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_Central\012");
                                    break;
                                case 11103: // Proj_Idaho_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS27_West\012");
                                    break;
                                case 11131: // Proj_Idaho_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_East\012");
                                    break;
                                case 11132: // Proj_Idaho_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_Central\012");
                                    break;
                                case 11133: // Proj_Idaho_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Idaho_CS83_West\012");
                                    break;
                                case 11201: // Proj_Illinois_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS27_East\012");
                                    break;
                                case 11202: // Proj_Illinois_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS27_West\012");
                                    break;
                                case 11231: // Proj_Illinois_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS83_East\012");
                                    break;
                                case 11232: // Proj_Illinois_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Illinois_CS83_West\012");
                                    break;
                                case 11301: // Proj_Indiana_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS27_East\012");
                                    break;
                                case 11302: // Proj_Indiana_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS27_West\012");
                                    break;
                                case 11331: // Proj_Indiana_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS83_East\012");
                                    break;
                                case 11332: // Proj_Indiana_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Indiana_CS83_West\012");
                                    break;
                                case 11401: // Proj_Iowa_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS27_North\012");
                                    break;
                                case 11402: // Proj_Iowa_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS27_South\012");
                                    break;
                                case 11431: // Proj_Iowa_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS83_North\012");
                                    break;
                                case 11432: // Proj_Iowa_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Iowa_CS83_South\012");
                                    break;
                                case 11501: // Proj_Kansas_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS27_North\012");
                                    break;
                                case 11502: // Proj_Kansas_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS27_South\012");
                                    break;
                                case 11531: // Proj_Kansas_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS83_North\012");
                                    break;
                                case 11532: // Proj_Kansas_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kansas_CS83_South\012");
                                    break;
                                case 11601: // Proj_Kentucky_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS27_North\012");
                                    break;
                                case 11602: // Proj_Kentucky_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS27_South\012");
                                    break;
                                case 11631: // Proj_Kentucky_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS83_North\012");
                                    break;
                                case 11632: // Proj_Kentucky_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Kentucky_CS83_South\012");
                                    break;
                                case 11701: // Proj_Louisiana_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS27_North\012");
                                    break;
                                case 11702: // Proj_Louisiana_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS27_South\012");
                                    break;
                                case 11731: // Proj_Louisiana_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS83_North\012");
                                    break;
                                case 11732: // Proj_Louisiana_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Louisiana_CS83_South\012");
                                    break;
                                case 11801: // Proj_Maine_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS27_East\012");
                                    break;
                                case 11802: // Proj_Maine_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS27_West\012");
                                    break;
                                case 11831: // Proj_Maine_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS83_East\012");
                                    break;
                                case 11832: // Proj_Maine_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maine_CS83_West\012");
                                    break;
                                case 11900: // Proj_Maryland_CS27					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maryland_CS27\012");
                                    break;
                                case 11930: // Proj_Maryland_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Maryland_CS83\012");
                                    break;
                                case 12001: // Proj_Massachusetts_CS27_Mainland			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS27_Mainland\012");
                                    break;
                                case 12002: // Proj_Massachusetts_CS27_Island			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS27_Island\012");
                                    break;
                                case 12031: // Proj_Massachusetts_CS83_Mainland			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS83_Mainland\012");
                                    break;
                                case 12032: // Proj_Massachusetts_CS83_Island			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Massachusetts_CS83_Island\012");
                                    break;
                                case 12101: // Proj_Michigan_State_Plane_East			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_East\012");
                                    break;
                                case 12102: // Proj_Michigan_State_Plane_Old_Central		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_Old_Central\012");
                                    break;
                                case 12103: // Proj_Michigan_State_Plane_West			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_State_Plane_West\012");
                                    break;
                                case 12111: // Proj_Michigan_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_North\012");
                                    break;
                                case 12112: // Proj_Michigan_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_Central\012");
                                    break;
                                case 12113: // Proj_Michigan_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS27_South\012");
                                    break;
                                case 12141: // Proj_Michigan_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_North\012");
                                    break;
                                case 12142: // Proj_Michigan_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_Central\012");
                                    break;
                                case 12143: // Proj_Michigan_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Michigan_CS83_South\012");
                                    break;
                                case 12201: // Proj_Minnesota_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_North\012");
                                    break;
                                case 12202: // Proj_Minnesota_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_Central\012");
                                    break;
                                case 12203: // Proj_Minnesota_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS27_South\012");
                                    break;
                                case 12231: // Proj_Minnesota_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_North\012");
                                    break;
                                case 12232: // Proj_Minnesota_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_Central\012");
                                    break;
                                case 12233: // Proj_Minnesota_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Minnesota_CS83_South\012");
                                    break;
                                case 12301: // Proj_Mississippi_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS27_East\012");
                                    break;
                                case 12302: // Proj_Mississippi_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS27_West\012");
                                    break;
                                case 12331: // Proj_Mississippi_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS83_East\012");
                                    break;
                                case 12332: // Proj_Mississippi_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Mississippi_CS83_West\012");
                                    break;
                                case 12401: // Proj_Missouri_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_East\012");
                                    break;
                                case 12402: // Proj_Missouri_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_Central\012");
                                    break;
                                case 12403: // Proj_Missouri_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS27_West\012");
                                    break;
                                case 12431: // Proj_Missouri_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_East\012");
                                    break;
                                case 12432: // Proj_Missouri_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_Central\012");
                                    break;
                                case 12433: // Proj_Missouri_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Missouri_CS83_West\012");
                                    break;
                                case 12501: // Proj_Montana_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_North\012");
                                    break;
                                case 12502: // Proj_Montana_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_Central\012");
                                    break;
                                case 12503: // Proj_Montana_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS27_South\012");
                                    break;
                                case 12530: // Proj_Montana_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Montana_CS83\012");
                                    break;
                                case 12601: // Proj_Nebraska_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS27_North\012");
                                    break;
                                case 12602: // Proj_Nebraska_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS27_South\012");
                                    break;
                                case 12630: // Proj_Nebraska_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nebraska_CS83\012");
                                    break;
                                case 12701: // Proj_Nevada_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_East\012");
                                    break;
                                case 12702: // Proj_Nevada_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_Central\012");
                                    break;
                                case 12703: // Proj_Nevada_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS27_West\012");
                                    break;
                                case 12731: // Proj_Nevada_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_East\012");
                                    break;
                                case 12732: // Proj_Nevada_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_Central\012");
                                    break;
                                case 12733: // Proj_Nevada_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Nevada_CS83_West\012");
                                    break;
                                case 12800: // Proj_New_Hampshire_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Hampshire_CS27\012");
                                    break;
                                case 12830: // Proj_New_Hampshire_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Hampshire_CS83\012");
                                    break;
                                case 12900: // Proj_New_Jersey_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Jersey_CS27\012");
                                    break;
                                case 12930: // Proj_New_Jersey_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Jersey_CS83\012");
                                    break;
                                case 13001: // Proj_New_Mexico_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_East\012");
                                    break;
                                case 13002: // Proj_New_Mexico_CS27_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_Central\012");
                                    break;
                                case 13003: // Proj_New_Mexico_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS27_West\012");
                                    break;
                                case 13031: // Proj_New_Mexico_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_East\012");
                                    break;
                                case 13032: // Proj_New_Mexico_CS83_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_Central\012");
                                    break;
                                case 13033: // Proj_New_Mexico_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Mexico_CS83_West\012");
                                    break;
                                case 13101: // Proj_New_York_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_East\012");
                                    break;
                                case 13102: // Proj_New_York_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_Central\012");
                                    break;
                                case 13103: // Proj_New_York_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_West\012");
                                    break;
                                case 13104: // Proj_New_York_CS27_Long_Island			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS27_Long_Island\012");
                                    break;
                                case 13131: // Proj_New_York_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_East\012");
                                    break;
                                case 13132: // Proj_New_York_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_Central\012");
                                    break;
                                case 13133: // Proj_New_York_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_West\012");
                                    break;
                                case 13134: // Proj_New_York_CS83_Long_Island			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_York_CS83_Long_Island\012");
                                    break;
                                case 13200: // Proj_North_Carolina_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Carolina_CS27\012");
                                    break;
                                case 13230: // Proj_North_Carolina_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Carolina_CS83\012");
                                    break;
                                case 13301: // Proj_North_Dakota_CS27_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS27_North\012");
                                    break;
                                case 13302: // Proj_North_Dakota_CS27_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS27_South\012");
                                    break;
                                case 13331: // Proj_North_Dakota_CS83_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS83_North\012");
                                    break;
                                case 13332: // Proj_North_Dakota_CS83_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_North_Dakota_CS83_South\012");
                                    break;
                                case 13401: // Proj_Ohio_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS27_North\012");
                                    break;
                                case 13402: // Proj_Ohio_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS27_South\012");
                                    break;
                                case 13431: // Proj_Ohio_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS83_North\012");
                                    break;
                                case 13432: // Proj_Ohio_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Ohio_CS83_South\012");
                                    break;
                                case 13501: // Proj_Oklahoma_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS27_North\012");
                                    break;
                                case 13502: // Proj_Oklahoma_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS27_South\012");
                                    break;
                                case 13531: // Proj_Oklahoma_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS83_North\012");
                                    break;
                                case 13532: // Proj_Oklahoma_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oklahoma_CS83_South\012");
                                    break;
                                case 13601: // Proj_Oregon_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS27_North\012");
                                    break;
                                case 13602: // Proj_Oregon_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS27_South\012");
                                    break;
                                case 13631: // Proj_Oregon_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS83_North\012");
                                    break;
                                case 13632: // Proj_Oregon_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Oregon_CS83_South\012");
                                    break;
                                case 13701: // Proj_Pennsylvania_CS27_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS27_North\012");
                                    break;
                                case 13702: // Proj_Pennsylvania_CS27_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS27_South\012");
                                    break;
                                case 13731: // Proj_Pennsylvania_CS83_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS83_North\012");
                                    break;
                                case 13732: // Proj_Pennsylvania_CS83_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Pennsylvania_CS83_South\012");
                                    break;
                                case 13800: // Proj_Rhode_Island_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Rhode_Island_CS27\012");
                                    break;
                                case 13830: // Proj_Rhode_Island_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Rhode_Island_CS83\012");
                                    break;
                                case 13901: // Proj_South_Carolina_CS27_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS27_North\012");
                                    break;
                                case 13902: // Proj_South_Carolina_CS27_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS27_South\012");
                                    break;
                                case 13930: // Proj_South_Carolina_CS83				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Carolina_CS83\012");
                                    break;
                                case 14001: // Proj_South_Dakota_CS27_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS27_North\012");
                                    break;
                                case 14002: // Proj_South_Dakota_CS27_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS27_South\012");
                                    break;
                                case 14031: // Proj_South_Dakota_CS83_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS83_North\012");
                                    break;
                                case 14032: // Proj_South_Dakota_CS83_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_South_Dakota_CS83_South\012");
                                    break;
                                case 14100: // Proj_Tennessee_CS27					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Tennessee_CS27\012");
                                    break;
                                case 14130: // Proj_Tennessee_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Tennessee_CS83\012");
                                    break;
                                case 14201: // Proj_Texas_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_North\012");
                                    break;
                                case 14202: // Proj_Texas_CS27_North_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_North_Central\012");
                                    break;
                                case 14203: // Proj_Texas_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_Central\012");
                                    break;
                                case 14204: // Proj_Texas_CS27_South_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_South_Central\012");
                                    break;
                                case 14205: // Proj_Texas_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS27_South\012");
                                    break;
                                case 14231: // Proj_Texas_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_North\012");
                                    break;
                                case 14232: // Proj_Texas_CS83_North_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_North_Central\012");
                                    break;
                                case 14233: // Proj_Texas_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_Central\012");
                                    break;
                                case 14234: // Proj_Texas_CS83_South_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_South_Central\012");
                                    break;
                                case 14235: // Proj_Texas_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Texas_CS83_South\012");
                                    break;
                                case 14301: // Proj_Utah_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_North\012");
                                    break;
                                case 14302: // Proj_Utah_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_Central\012");
                                    break;
                                case 14303: // Proj_Utah_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS27_South\012");
                                    break;
                                case 14331: // Proj_Utah_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_North\012");
                                    break;
                                case 14332: // Proj_Utah_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_Central\012");
                                    break;
                                case 14333: // Proj_Utah_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Utah_CS83_South\012");
                                    break;
                                case 14400: // Proj_Vermont_CS27					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Vermont_CS27\012");
                                    break;
                                case 14430: // Proj_Vermont_CS83					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Vermont_CS83\012");
                                    break;
                                case 14501: // Proj_Virginia_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS27_North\012");
                                    break;
                                case 14502: // Proj_Virginia_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS27_South\012");
                                    break;
                                case 14531: // Proj_Virginia_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS83_North\012");
                                    break;
                                case 14532: // Proj_Virginia_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Virginia_CS83_South\012");
                                    break;
                                case 14601: // Proj_Washington_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS27_North\012");
                                    break;
                                case 14602: // Proj_Washington_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS27_South\012");
                                    break;
                                case 14631: // Proj_Washington_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS83_North\012");
                                    break;
                                case 14632: // Proj_Washington_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Washington_CS83_South\012");
                                    break;
                                case 14701: // Proj_West_Virginia_CS27_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS27_North\012");
                                    break;
                                case 14702: // Proj_West_Virginia_CS27_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS27_South\012");
                                    break;
                                case 14731: // Proj_West_Virginia_CS83_North			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS83_North\012");
                                    break;
                                case 14732: // Proj_West_Virginia_CS83_South			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_West_Virginia_CS83_South\012");
                                    break;
                                case 14801: // Proj_Wisconsin_CS27_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_North\012");
                                    break;
                                case 14802: // Proj_Wisconsin_CS27_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_Central\012");
                                    break;
                                case 14803: // Proj_Wisconsin_CS27_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS27_South\012");
                                    break;
                                case 14831: // Proj_Wisconsin_CS83_North				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_North\012");
                                    break;
                                case 14832: // Proj_Wisconsin_CS83_Central				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_Central\012");
                                    break;
                                case 14833: // Proj_Wisconsin_CS83_South				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wisconsin_CS83_South\012");
                                    break;
                                case 14901: // Proj_Wyoming_CS27_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_East\012");
                                    break;
                                case 14902: // Proj_Wyoming_CS27_East_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_East_Central\012");
                                    break;
                                case 14903: // Proj_Wyoming_CS27_West_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_West_Central\012");
                                    break;
                                case 14904: // Proj_Wyoming_CS27_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS27_West\012");
                                    break;
                                case 14931: // Proj_Wyoming_CS83_East				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_East\012");
                                    break;
                                case 14932: // Proj_Wyoming_CS83_East_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_East_Central\012");
                                    break;
                                case 14933: // Proj_Wyoming_CS83_West_Central			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_West_Central\012");
                                    break;
                                case 14934: // Proj_Wyoming_CS83_West				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Wyoming_CS83_West\012");
                                    break;
                                case 15001: // Proj_Alaska_CS27_1					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_1\012");
                                    break;
                                case 15002: // Proj_Alaska_CS27_2					
                                    fprintf(file_out, "ProjectionGeoKey: ProjectionGeoKey\012");
                                    break;
                                case 15003: // Proj_Alaska_CS27_3					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_3\012");
                                    break;
                                case 15004: // Proj_Alaska_CS27_4					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_4\012");
                                    break;
                                case 15005: // Proj_Alaska_CS27_5					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_5\012");
                                    break;
                                case 15006: // Proj_Alaska_CS27_6					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_6\012");
                                    break;
                                case 15007: // Proj_Alaska_CS27_7					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_7\012");
                                    break;
                                case 15008: // Proj_Alaska_CS27_8					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_8\012");
                                    break;
                                case 15009: // Proj_Alaska_CS27_9					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_9\012");
                                    break;
                                case 15010: // Proj_Alaska_CS27_10					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS27_10\012");
                                    break;
                                case 15031: // Proj_Alaska_CS83_1					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_1\012");
                                    break;
                                case 15032: // Proj_Alaska_CS83_2					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_2\012");
                                    break;
                                case 15033: // Proj_Alaska_CS83_3					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_3\012");
                                    break;
                                case 15034: // Proj_Alaska_CS83_4					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_4\012");
                                    break;
                                case 15035: // Proj_Alaska_CS83_5					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_5\012");
                                    break;
                                case 15036: // Proj_Alaska_CS83_6					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_6\012");
                                    break;
                                case 15037: // Proj_Alaska_CS83_7					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_7\012");
                                    break;
                                case 15038: // Proj_Alaska_CS83_8					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_8\012");
                                    break;
                                case 15039: // Proj_Alaska_CS83_9					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_9\012");
                                    break;
                                case 15040: // Proj_Alaska_CS83_10					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Alaska_CS83_10\012");
                                    break;
                                case 15101: // Proj_Hawaii_CS27_1					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_1\012");
                                    break;
                                case 15102: // Proj_Hawaii_CS27_2					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_2\012");
                                    break;
                                case 15103: // Proj_Hawaii_CS27_3					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_3\012");
                                    break;
                                case 15104: // Proj_Hawaii_CS27_4					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_4\012");
                                    break;
                                case 15105: // Proj_Hawaii_CS27_5					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS27_5\012");
                                    break;
                                case 15131: // Proj_Hawaii_CS83_1					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_1\012");
                                    break;
                                case 15132: // Proj_Hawaii_CS83_2					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_2\012");
                                    break;
                                case 15133: // Proj_Hawaii_CS83_3					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_3\012");
                                    break;
                                case 15134: // Proj_Hawaii_CS83_4					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_4\012");
                                    break;
                                case 15135: // Proj_Hawaii_CS83_5					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Hawaii_CS83_5\012");
                                    break;
                                case 15201: // Proj_Puerto_Rico_CS27				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Puerto_Rico_CS27\012");
                                    break;
                                case 15202: // Proj_St_Croix					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_St_Croix\012");
                                    break;
                                case 15230: // Proj_Puerto_Rico_Virgin_Is				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Puerto_Rico_Virgin_Is\012");
                                    break;
                                case 15914: // Proj_BLM_14N_feet					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_BLM_14N_feet\012");
                                    break;
                                case 15915: // Proj_BLM_15N_feet					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_BLM_15N_feet\012");
                                    break;
                                case 15916: // Proj_BLM_16N_feet					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_BLM_16N_feet\012");
                                    break;
                                case 15917: // Proj_BLM_17N_feet					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_BLM_17N_feet\012");
                                    break;
                                case 17333: // Proj_SWEREF99_TM		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_SWEREF99_TM\012");
                                    break;
                                case 17348: // Proj_Map_Grid_of_Australia_48			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_48\012");
                                    break;
                                case 17349: // Proj_Map_Grid_of_Australia_49			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_49\012");
                                    break;
                                case 17350: // Proj_Map_Grid_of_Australia_50			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_50\012");
                                    break;
                                case 17351: // Proj_Map_Grid_of_Australia_51			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_51\012");
                                    break;
                                case 17352: // Proj_Map_Grid_of_Australia_52			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_52\012");
                                    break;
                                case 17353: // Proj_Map_Grid_of_Australia_53			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_53\012");
                                    break;
                                case 17354: // Proj_Map_Grid_of_Australia_54			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_54\012");
                                    break;
                                case 17355: // Proj_Map_Grid_of_Australia_55			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_55\012");
                                    break;
                                case 17356: // Proj_Map_Grid_of_Australia_56			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_56\012");
                                    break;
                                case 17357: // Proj_Map_Grid_of_Australia_57			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_57\012");
                                    break;
                                case 17358: // Proj_Map_Grid_of_Australia_58			
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Map_Grid_of_Australia_58\012");
                                    break;
                                case 17448: // Proj_Australian_Map_Grid_48				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_48\012");
                                    break;
                                case 17449: // Proj_Australian_Map_Grid_49				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_49\012");
                                    break;
                                case 17450: // Proj_Australian_Map_Grid_50				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_50\012");
                                    break;
                                case 17451: // Proj_Australian_Map_Grid_51				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_51\012");
                                    break;
                                case 17452: // Proj_Australian_Map_Grid_52				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_52\012");
                                    break;
                                case 17453: // Proj_Australian_Map_Grid_53				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_53\012");
                                    break;
                                case 17454: // Proj_Australian_Map_Grid_54				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_54\012");
                                    break;
                                case 17455: // Proj_Australian_Map_Grid_55				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_55\012");
                                    break;
                                case 17456: // Proj_Australian_Map_Grid_56				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_56\012");
                                    break;
                                case 17457: // Proj_Australian_Map_Grid_57				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_57\012");
                                    break;
                                case 17458: // Proj_Australian_Map_Grid_58				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Australian_Map_Grid_58\012");
                                    break;
                                case 18031: // Proj_Argentina_1					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_1\012");
                                    break;
                                case 18032: // Proj_Argentina_2					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_2\012");
                                    break;
                                case 18033: // Proj_Argentina_3					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_3\012");
                                    break;
                                case 18034: // Proj_Argentina_4					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_4\012");
                                    break;
                                case 18035: // Proj_Argentina_5					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_5\012");
                                    break;
                                case 18036: // Proj_Argentina_6					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_6\012");
                                    break;
                                case 18037: // Proj_Argentina_7					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Argentina_7\012");
                                    break;
                                case 18051: // Proj_Colombia_3W					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_3W\012");
                                    break;
                                case 18052: // Proj_Colombia_Bogota				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_Bogota\012");
                                    break;
                                case 18053: // Proj_Colombia_3E					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_3E\012");
                                    break;
                                case 18054: // Proj_Colombia_6E					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Colombia_6E\012");
                                    break;
                                case 18072: // Proj_Egypt_Red_Belt					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Egypt_Red_Belt\012");
                                    break;
                                case 18073: // Proj_Egypt_Purple_Belt				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Egypt_Purple_Belt\012");
                                    break;
                                case 18074: // Proj_Extended_Purple_Belt				
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Extended_Purple_Belt\012");
                                    break;
                                case 18141: // Proj_New_Zealand_North_Island_Nat_Grid		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Zealand_North_Island_Nat_Grid\012");
                                    break;
                                case 18142: // Proj_New_Zealand_South_Island_Nat_Grid		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_New_Zealand_South_Island_Nat_Grid\012");
                                    break;
                                case 19900: // Proj_Bahrain_Grid					
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Bahrain_Grid\012");
                                    break;
                                case 19905: // Proj_Netherlands_E_Indies_Equatorial		
                                    fprintf(file_out, "ProjectionGeoKey: Proj_Netherlands_E_Indies_Equatorial\012");
                                    break;
                                case 19912: // Proj_RSO_Borneo
                                    fprintf(file_out, "ProjectionGeoKey: Proj_RSO_Borneo\012");
                                    break;
                                default:
                                    fprintf(file_out, "ProjectionGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                                }
                            }
                            break;
                        case 3075: // ProjCoordTransGeoKey
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 1: // CT_TransverseMercator
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_TransverseMercator\012");
                                break;
                            case 2: // CT_TransvMercator_Modified_Alaska
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_TransvMercator_Modified_Alaska\012");
                                break;
                            case 3: // CT_ObliqueMercator
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator\012");
                                break;
                            case 4: // CT_ObliqueMercator_Laborde
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Laborde\012");
                                break;
                            case 5: // CT_ObliqueMercator_Rosenmund
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Rosenmund\012");
                                break;
                            case 6: // CT_ObliqueMercator_Spherical
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueMercator_Spherical\012");
                                break;
                            case 7: // CT_Mercator
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Mercator\012");
                                break;
                            case 8: // CT_LambertConfConic_2SP
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertConfConic_2SP\012");
                                break;
                            case 9: // CT_LambertConfConic_Helmert
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertConfConic_Helmert\012");
                                break;
                            case 10: // CT_LambertAzimEqualArea
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_LambertAzimEqualArea\012");
                                break;
                            case 11: // CT_AlbersEqualArea
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_AlbersEqualArea\012");
                                break;
                            case 12: // CT_AzimuthalEquidistant
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_AzimuthalEquidistant\012");
                                break;
                            case 13: // CT_EquidistantConic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_EquidistantConic\012");
                                break;
                            case 14: // CT_Stereographic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Stereographic\012");
                                break;
                            case 15: // CT_PolarStereographic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_PolarStereographic\012");
                                break;
                            case 16: // CT_ObliqueStereographic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_ObliqueStereographic\012");
                                break;
                            case 17: // CT_Equirectangular
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Equirectangular\012");
                                break;
                            case 18: // CT_CassiniSoldner
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_CassiniSoldner\012");
                                break;
                            case 19: // CT_Gnomonic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Gnomonic\012");
                                break;
                            case 20: // CT_MillerCylindrical
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_MillerCylindrical\012");
                                break;
                            case 21: // CT_Orthographic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Orthographic\012");
                                break;
                            case 22: // CT_Polyconic
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Polyconic\012");
                                break;
                            case 23: // CT_Robinson
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Robinson\012");
                                break;
                            case 24: // CT_Sinusoidal
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_Sinusoidal\012");
                                break;
                            case 25: // CT_VanDerGrinten
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_VanDerGrinten\012");
                                break;
                            case 26: // CT_NewZealandMapGrid
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_NewZealandMapGrid\012");
                                break;
                            case 27: // CT_TransvMercator_SouthOriented
                                fprintf(file_out, "ProjCoordTransGeoKey: CT_TransvMercator_SouthOriented\012");
                                break;
                            default:
                                fprintf(file_out, "ProjCoordTransGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 3076: // ProjLinearUnitsGeoKey
                            horizontal_units = lasreader->header.vlr_geo_key_entries[j].value_offset;
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 9001: // Linear_Meter
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Meter\012");
                                break;
                            case 9002: // Linear_Foot
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot\012");
                                break;
                            case 9003: // Linear_Foot_US_Survey
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_US_Survey\012");
                                break;
                            case 9004: // Linear_Foot_Modified_American
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Modified_American\012");
                                break;
                            case 9005: // Linear_Foot_Clarke
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Clarke\012");
                                break;
                            case 9006: // Linear_Foot_Indian
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Foot_Indian\012");
                                break;
                            case 9007: // Linear_Link
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link\012");
                                break;
                            case 9008: // Linear_Link_Benoit
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link_Benoit\012");
                                break;
                            case 9009: // Linear_Link_Sears
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Link_Sears\012");
                                break;
                            case 9010: // Linear_Chain_Benoit
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Chain_Benoit\012");
                                break;
                            case 9011: // Linear_Chain_Sears
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Chain_Sears\012");
                                break;
                            case 9012: // Linear_Yard_Sears
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Yard_Sears\012");
                                break;
                            case 9013: // Linear_Yard_Indian
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Yard_Indian\012");
                                break;
                            case 9014: // Linear_Fathom
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Fathom\012");
                                break;
                            case 9015: // Linear_Mile_International_Nautical
                                fprintf(file_out, "ProjLinearUnitsGeoKey: Linear_Mile_International_Nautical\012");
                                break;
                            default:
                                fprintf(file_out, "ProjLinearUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        case 3077: // ProjLinearUnitSizeGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjLinearUnitSizeGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3078: // ProjStdParallel1GeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjStdParallel1GeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3079: // ProjStdParallel2GeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjStdParallel2GeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3080: // ProjNatOriginLongGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjNatOriginLongGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3081: // ProjNatOriginLatGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjNatOriginLatGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3082: // ProjFalseEastingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseEastingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3083: // ProjFalseNorthingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseNorthingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3084: // ProjFalseOriginLongGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseOriginLongGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3085: // ProjFalseOriginLatGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseOriginLatGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3086: // ProjFalseOriginEastingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseOriginEastingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3087: // ProjFalseOriginNorthingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjFalseOriginNorthingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3088: // ProjCenterLongGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjCenterLongGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3089: // ProjCenterLatGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjCenterLatGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3090: // ProjCenterEastingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjCenterEastingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3091: // ProjCenterNorthingGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjCenterNorthingGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3092: // ProjScaleAtNatOriginGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjScaleAtNatOriginGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3093: // ProjScaleAtCenterGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjScaleAtCenterGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3094: // ProjAzimuthAngleGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjAzimuthAngleGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 3095: // ProjStraightVertPoleLongGeoKey
                            if (lasreader->header.vlr_geo_double_params)
                            {
                                fprintf(file_out, "ProjStraightVertPoleLongGeoKey: %.10g\012", lasreader->header.vlr_geo_double_params[lasreader->header.vlr_geo_key_entries[j].value_offset]);
                            }
                            break;
                        case 4096: // VerticalCSTypeGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 1127: // VertCS_Canadian_Geodetic_Vertical_Datum_2013
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Canadian_Geodetic_Vertical_Datum_2013\012");
                                break;
                            case 5001: // VertCS_Airy_1830_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Airy_1830_ellipsoid\012");
                                break;
                            case 5002: // VertCS_Airy_Modified_1849_ellipsoid 
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Airy_Modified_1849_ellipsoid\012");
                                break;
                            case 5003: // VertCS_ANS_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_ANS_ellipsoid\012");
                                break;
                            case 5004: // VertCS_Bessel_1841_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_1841_ellipsoid\012");
                                break;
                            case 5005: // VertCS_Bessel_Modified_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_Modified_ellipsoid\012");
                                break;
                            case 5006: // VertCS_Bessel_Namibia_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Bessel_Namibia_ellipsoid\012");
                                break;
                            case 5007: // VertCS_Clarke_1858_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1858_ellipsoid\012");
                                break;
                            case 5008: // VertCS_Clarke_1866_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1866_ellipsoid\012");
                                break;
                            case 5010: // VertCS_Clarke_1880_Benoit_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_Benoit_ellipsoid\012");
                                break;
                            case 5011: // VertCS_Clarke_1880_IGN_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_IGN_ellipsoid\012");
                                break;
                            case 5012: // VertCS_Clarke_1880_RGS_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_RGS_ellipsoid\012");
                                break;
                            case 5013: // VertCS_Clarke_1880_Arc_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_Arc_ellipsoid\012");
                                break;
                            case 5014: // VertCS_Clarke_1880_SGA_1922_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Clarke_1880_SGA_1922_ellipsoid\012");
                                break;
                            case 5015: // VertCS_Everest_1830_1937_Adjustment_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1937_Adjustment_ellipsoid\012");
                                break;
                            case 5016: // VertCS_Everest_1830_1967_Definition_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1967_Definition_ellipsoid\012");
                                break;
                            case 5017: // VertCS_Everest_1830_1975_Definition_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_1975_Definition_ellipsoid\012");
                                break;
                            case 5018: // VertCS_Everest_1830_Modified_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Everest_1830_Modified_ellipsoid\012");
                                break;
                            case 5019: // VertCS_GRS_1980_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_GRS_1980_ellipsoid\012");
                                break;
                            case 5020: // VertCS_Helmert_1906_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Helmert_1906_ellipsoid\012");
                                break;
                            case 5021: // VertCS_INS_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_INS_ellipsoid\012");
                                break;
                            case 5022: // VertCS_International_1924_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_International_1924_ellipsoid\012");
                                break;
                            case 5023: // VertCS_International_1967_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_International_1967_ellipsoid\012");
                                break;
                            case 5024: // VertCS_Krassowsky_1940_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Krassowsky_1940_ellipsoid\012");
                                break;
                            case 5025: // VertCS_NWL_9D_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_NWL_9D_ellipsoid\012");
                                break;
                            case 5026: // VertCS_NWL_10D_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_NWL_10D_ellipsoid\012");
                                break;
                            case 5027: // VertCS_Plessis_1817_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Plessis_1817_ellipsoid\012");
                                break;
                            case 5028: // VertCS_Struve_1860_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Struve_1860_ellipsoid\012");
                                break;
                            case 5029: // VertCS_War_Office_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_War_Office_ellipsoid\012");
                                break;
                            case 5030: // VertCS_WGS_84_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_WGS_84_ellipsoid\012");
                                break;
                            case 5031: // VertCS_GEM_10C_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_GEM_10C_ellipsoid\012");
                                break;
                            case 5032: // VertCS_OSU86F_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_OSU86F_ellipsoid\012");
                                break;
                            case 5033: // VertCS_OSU91A_ellipsoid
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_OSU91A_ellipsoid\012");
                                break;
                            case 5101: // VertCS_Newlyn
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Newlyn\012");
                                break;
                            case 5102: // VertCS_North_American_Vertical_Datum_1929
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_North_American_Vertical_Datum_1929\012");
                                break;
                            case 5103: // VertCS_North_American_Vertical_Datum_1988
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_North_American_Vertical_Datum_1988\012");
                                break;
                            case 5104: // VertCS_Yellow_Sea_1956
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Yellow_Sea_1956\012");
                                break;
                            case 5105: // VertCS_Baltic_Sea
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Baltic_Sea\012");
                                break;
                            case 5106: // VertCS_Caspian_Sea
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Caspian_Sea\012");
                                break;
                            case 5114: // VertCS_Canadian_Geodetic_Vertical_Datum_1928
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Canadian_Geodetic_Vertical_Datum_1928\012");
                                break;
                            case 5206: // VertCS_Dansk_Vertikal_Reference_1990
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_Dansk_Vertikal_Reference_1990\012");
                                break;
                            case 5215: // VertCS_European_Vertical_Reference_Frame_2007
                                fprintf(file_out, "VerticalCSTypeGeoKey: VertCS_European_Vertical_Reference_Frame_2007\012");
                                break;
                            case 5701: // ODN height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: ODN height (Reserved EPSG)\012");
                                break;
                            case 5702: // NGVD29 height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: NGVD29 height (Reserved EPSG)\012");
                                break;
                            case 5703: // NAVD88 height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: NAVD88 height (Reserved EPSG)\012");
                                break;
                            case 5704: // Yellow Sea (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: Yellow Sea (Reserved EPSG)\012");
                                break;
                            case 5705: // Baltic height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: Baltic height (Reserved EPSG)\012");
                                break;
                            case 5706: // Caspian depth (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: Caspian depth (Reserved EPSG)\012");
                                break;
                            case 5707: // NAP height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: NAP height (Reserved EPSG)\012");
                                break;
                            case 5710: // Oostende height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: Oostende height (Reserved EPSG)\012");
                                break;
                            case 5711: // AHD height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: AHD height (Reserved EPSG)\012");
                                break;
                            case 5712: // AHD (Tasmania) height (Reserved EPSG)
                                fprintf(file_out, "VerticalCSTypeGeoKey: AHD (Tasmania) height (Reserved EPSG)\012");
                                break;
                            case 5776: // Norway Normal Null 1954
                                fprintf(file_out, "VerticalCSTypeGeoKey: Norway Normal Null 1954\012");
                                break;
                            case 5783: // Deutches Haupthohennetz 1992
                                fprintf(file_out, "VerticalCSTypeGeoKey: Deutsches Haupthoehennetz 1992\012");
                                break;
                            case 5941: // Norway Normal Null 2000
                                fprintf(file_out, "VerticalCSTypeGeoKey: Norway Normal Null 2000\012");
                                break;
                            case 6647: // Canadian Geodetic Vertical Datum of 2013
                                fprintf(file_out, "VerticalCSTypeGeoKey: Canadian Geodetic Vertical Datum of 2013\012");
                                break;
                            case 7837: // Deutches Haupthohennetz 2016
                                fprintf(file_out, "VerticalCSTypeGeoKey: Deutsches Haupthoehennetz 2016\012");
                                break;
                            default:
                                if (pGeoProjectionConverter->set_VerticalCSTypeGeoKey(lasreader->header.vlr_geo_key_entries[j].value_offset, printstring))
                                {
                                    fprintf(file_out, "VerticalCSTypeGeoKey: %s\012", printstring);
                                }
                                else
                                {
                                    fprintf(file_out, "VerticalCSTypeGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                                }
                            }
                            break;
                        case 4097: // VerticalCitationGeoKey
                            if (lasreader->header.vlr_geo_ascii_params)
                            {
                                char dummy[256];
                                strncpy(dummy, &(lasreader->header.vlr_geo_ascii_params[lasreader->header.vlr_geo_key_entries[j].value_offset]), lasreader->header.vlr_geo_key_entries[j].count);
                                dummy[lasreader->header.vlr_geo_key_entries[j].count - 1] = '\0';
                                fprintf(file_out, "VerticalCitationGeoKey: %s\012", dummy);
                            }
                            break;
                        case 4098: // VerticalDatumGeoKey 
                            fprintf(file_out, "VerticalDatumGeoKey: Vertical Datum Codes %d\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            break;
                        case 4099: // VerticalUnitsGeoKey 
                            switch (lasreader->header.vlr_geo_key_entries[j].value_offset)
                            {
                            case 9001: // Linear_Meter
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Meter\012");
                                break;
                            case 9002: // Linear_Foot
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot\012");
                                break;
                            case 9003: // Linear_Foot_US_Survey
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_US_Survey\012");
                                break;
                            case 9004: // Linear_Foot_Modified_American
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Modified_American\012");
                                break;
                            case 9005: // Linear_Foot_Clarke
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Clarke\012");
                                break;
                            case 9006: // Linear_Foot_Indian
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Foot_Indian\012");
                                break;
                            case 9007: // Linear_Link
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link\012");
                                break;
                            case 9008: // Linear_Link_Benoit
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link_Benoit\012");
                                break;
                            case 9009: // Linear_Link_Sears
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Link_Sears\012");
                                break;
                            case 9010: // Linear_Chain_Benoit
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Chain_Benoit\012");
                                break;
                            case 9011: // Linear_Chain_Sears
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Chain_Sears\012");
                                break;
                            case 9012: // Linear_Yard_Sears
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Yard_Sears\012");
                                break;
                            case 9013: // Linear_Yard_Indian
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Yard_Indian\012");
                                break;
                            case 9014: // Linear_Fathom
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Fathom\012");
                                break;
                            case 9015: // Linear_Mile_International_Nautical
                                fprintf(file_out, "VerticalUnitsGeoKey: Linear_Mile_International_Nautical\012");
                                break;
                            default:
                                fprintf(file_out, "VerticalUnitsGeoKey: look-up for %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].value_offset);
                            }
                            break;
                        default:
                            fprintf(file_out, "key ID %d not implemented\012", lasreader->header.vlr_geo_key_entries[j].key_id);
                        }
                    }
                }
            }
            else if (lasheader->vlrs[i].record_id == 34736) // GeoDoubleParamsTag
            {
                fprintf(file_out, "    GeoDoubleParamsTag (number of doubles %d)\012", lasreader->header.vlrs[i].record_length_after_header / 8);
                fprintf(file_out, "      ");
                for (int j = 0; j < lasreader->header.vlrs[i].record_length_after_header / 8; j++)
                {
                    fprintf(file_out, "%lg ", lasheader->vlr_geo_double_params[j]);
                }
                fprintf(file_out, "\012");
            }
            else if (lasheader->vlrs[i].record_id == 34737) // GeoAsciiParamsTag
            {
                fprintf(file_out, "    GeoAsciiParamsTag (number of characters %d)\012", lasreader->header.vlrs[i].record_length_after_header);
                fprintf(file_out, "      ");
                for (int j = 0; j < lasreader->header.vlrs[i].record_length_after_header; j++)
                {
                    if (lasheader->vlr_geo_ascii_params[j] >= ' ')
                    {
                        fprintf(file_out, "%c", lasheader->vlr_geo_ascii_params[j]);
                    }
                    else
                    {
                        fprintf(file_out, " ");
                    }
                }
                fprintf(file_out, "\012");
            }
            else if (lasheader->vlrs[i].record_id == 2111) // WKT OGC MATH TRANSFORM
            {
                fprintf(file_out, "    WKT OGC MATH TRANSFORM:\012");
                fprintf(file_out, "    %s\012", lasreader->header.vlrs[i].data);
            }
            else if (lasheader->vlrs[i].record_id == 2112) // WKT OGC COORDINATE SYSTEM
            {
                fprintf(file_out, "    WKT OGC COORDINATE SYSTEM:\012");
                fprintf(file_out, "    %s\012", lasreader->header.vlrs[i].data);
            }
        }
        else if ((strcmp(lasheader->vlrs[i].user_id, "LASF_Spec") == 0) && (lasheader->vlrs[i].data != 0))
        {
            if (lasheader->vlrs[i].record_id == 0) // ClassificationLookup
            {
                LASvlr_classification* vlr_classification = (LASvlr_classification*)lasheader->vlrs[i].data;
                int num = lasheader->vlrs[i].record_length_after_header / sizeof(LASvlr_classification);
                for (int j = 0; j < num; j++)
                {
                    fprintf(file_out, "    %d %.15s", vlr_classification[j].class_number, vlr_classification[j].description);
                }
                if (num) fprintf(file_out, "\012");
            }
            else if (lasheader->vlrs[i].record_id == 2) // Histogram
            {
            }
            else if (lasheader->vlrs[i].record_id == 3) // TextAreaDescription
            {
                fprintf(file_out, "    ");
                for (int j = 0; j < lasheader->vlrs[i].record_length_after_header; j++)
                {
                    if (lasheader->vlrs[i].data[j] != '\0')
                    {
                        fprintf(file_out, "%c", lasheader->vlrs[i].data[j]);
                    }
                    else
                    {
                        fprintf(file_out, " ");
                    }
                }
                fprintf(file_out, "\012");
            }
            else if (lasheader->vlrs[i].record_id == 4) // ExtraBytes
            {
                const char* name_table[10] = { "unsigned char", "char", "unsigned short", "short", "unsigned long", "long", "unsigned long long", "long long", "float", "double" };
                fprintf(file_out, "    Extra Byte Descriptions\012");
                for (int j = 0; j < lasheader->vlrs[i].record_length_after_header; j += 192)
                {
                    if (lasheader->vlrs[i].data[j + 2])
                    {
                        int type = ((I32)(lasheader->vlrs[i].data[j + 2]) - 1) % 10;
                        int dim = ((I32)(lasheader->vlrs[i].data[j + 2]) - 1) / 10 + 1;
                        if (file_out)
                        {
                            fprintf(file_out, "      data type: %d (%s), name \"%s\", description: \"%s\"", (I32)(lasheader->vlrs[i].data[j + 2]), name_table[type], (char*)(lasheader->vlrs[i].data + j + 4), (char*)(lasheader->vlrs[i].data + j + 160));
                            if (lasheader->vlrs[i].data[j + 3] & 0x02) // if min is set
                            {
                                fprintf(file_out, ", min:");
                                for (int k = 0; k < dim; k++)
                                {
                                    if (type < 8)
                                    {
#ifdef _WIN32
                                        fprintf(file_out, " %I64d", ((I64*)(lasheader->vlrs[i].data + j + 64))[k]);
#else
                                        fprintf(file_out, ", %lld", ((I64*)(lasheader->vlrs[i].data + j + 64))[k]);
#endif
                                    }
                                    else
                                    {
                                        fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 64))[k]);
                                    }
                                }
                            }
                            if (lasheader->vlrs[i].data[j + 3] & 0x04) // if max is set
                            {
                                fprintf(file_out, ", max:");
                                for (int k = 0; k < dim; k++)
                                {
                                    if (type < 8)
                                    {
#ifdef _WIN32
                                        fprintf(file_out, " %I64d", ((I64*)(lasheader->vlrs[i].data + j + 88))[k]);
#else
                                        fprintf(file_out, ", %lld", ((I64*)(lasheader->vlrs[i].data + j + 88))[k]);
#endif
                                    }
                                    else
                                    {
                                        fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 88))[k]);
                                    }
                                }
                            }
                            if (lasheader->vlrs[i].data[j + 3] & 0x08) // if scale is set
                            {
                                fprintf(file_out, ", scale:");
                                for (int k = 0; k < dim; k++)
                                {
                                    fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 112))[k]);
                                }
                            }
                            else
                            {
                                fprintf(file_out, ", scale: 1 (not set)");
                            }
                            if (lasheader->vlrs[i].data[j + 3] & 0x10) // if offset is set
                            {
                                fprintf(file_out, ", offset:");
                                for (int k = 0; k < dim; k++)
                                {
                                    fprintf(file_out, " %g", ((F64*)(lasheader->vlrs[i].data + j + 136))[k]);
                                }
                            }
                            else
                            {
                                fprintf(file_out, ", offset: 0 (not set)");
                            }
                            fprintf(file_out, "\012");
                        }
                    }
                    else
                    {
                        fprintf(file_out, "      data type: 0 (untyped bytes), size: %d\012", lasheader->vlrs[i].data[j + 3]);
                    }
                }
            }
            else if ((lasheader->vlrs[i].record_id >= 100) && (lasheader->vlrs[i].record_id < 355)) // WavePacketDescriptor
            {
                LASvlr_wave_packet_descr* vlr_wave_packet_descr = (LASvlr_wave_packet_descr*)lasheader->vlrs[i].data;
                fprintf(file_out, "  index %d bits/sample %d compression %d samples %u temporal %u gain %lg, offset %lg\012", lasheader->vlrs[i].record_id - 99, vlr_wave_packet_descr->getBitsPerSample(), vlr_wave_packet_descr->getCompressionType(), vlr_wave_packet_descr->getNumberOfSamples(), vlr_wave_packet_descr->getTemporalSpacing(), vlr_wave_packet_descr->getDigitizerGain(), vlr_wave_packet_descr->getDigitizerOffset());
            }
        }
        else if ((strcmp(lasheader->vlrs[i].user_id, "Raster LAZ") == 0) && (lasheader->vlrs[i].record_id == 7113))
        {
            LASvlrRasterLAZ vlrRasterLAZ;
            if (vlrRasterLAZ.set_payload(lasheader->vlrs[i].data, lasheader->vlrs[i].record_length_after_header))
            {
                fprintf(file_out, "    ncols %6d\012", vlrRasterLAZ.ncols);
                fprintf(file_out, "    nrows %6d\012", vlrRasterLAZ.nrows);
                fprintf(file_out, "    llx   %.10g\012", vlrRasterLAZ.llx);
                fprintf(file_out, "    lly   %.10g\012", vlrRasterLAZ.lly);
                fprintf(file_out, "    stepx    %g\012", vlrRasterLAZ.stepx);
                fprintf(file_out, "    stepy    %g\012", vlrRasterLAZ.stepy);
                if (vlrRasterLAZ.sigmaxy)
                {
                    fprintf(file_out, "    sigmaxy %g\012", vlrRasterLAZ.sigmaxy);
                }
                else
                {
                    fprintf(file_out, "    sigmaxy <not set>\012");
                }
            }
            else
            {
                fprintf(file_out, "WARNING: corrupt RasterLAZ VLR\n");
            }
        }
    }
}

if (file_out && !no_variable_header)
{
    for (int i = 0; i < (int)lasheader->number_of_extended_variable_length_records; i++)
    {
        fprintf(file_out, "extended variable length header record %d of %d:\012", i + 1, (int)lasheader->number_of_extended_variable_length_records);
        fprintf(file_out, "  reserved             %d\012", lasreader->header.evlrs[i].reserved);
        fprintf(file_out, "  user ID              '%.16s'\012", lasreader->header.evlrs[i].user_id);
        fprintf(file_out, "  record ID            %d\012", lasreader->header.evlrs[i].record_id);
#ifdef _WIN32
        fprintf(file_out, "  length after header  %I64d\012", lasreader->header.evlrs[i].record_length_after_header);
#else
        fprintf(file_out, "  length after header  %lld\012", lasreader->header.evlrs[i].record_length_after_header);
#endif
        fprintf(file_out, "  description          '%.32s'\012", lasreader->header.evlrs[i].description);
        if (strcmp(lasheader->evlrs[i].user_id, "LASF_Projection") == 0)
        {
            if (lasheader->evlrs[i].record_id == 2111) // OGC MATH TRANSFORM WKT
            {
                fprintf(file_out, "    OGC MATH TRANSFORM WKT:\012");
                fprintf(file_out, "    %s\012", lasreader->header.evlrs[i].data);
            }
            else if (lasheader->evlrs[i].record_id == 2112) // OGC COORDINATE SYSTEM WKT
            {
                fprintf(file_out, "    OGC COORDINATE SYSTEM WKT:\012");
                fprintf(file_out, "    %s\012", lasreader->header.evlrs[i].data);
            }
        }
    }
}

if (file_out && !no_variable_header)
{
    const LASindex* index = lasreader->get_index();
    if (index)
    {
        fprintf(file_out, "has spatial indexing LAX file\012"); // index->start, index->end, index->full, index->total, index->cells);
    }
}

if (file_out && !no_header)
{
    if (lasheader->user_data_after_header_size) fprintf(file_out, "the header is followed by %u user-defined bytes\012", lasheader->user_data_after_header_size);

    if (lasheader->laszip)
    {
        fprintf(file_out, "LASzip compression (version %d.%dr%d c%d", lasheader->laszip->version_major, lasheader->laszip->version_minor, lasheader->laszip->version_revision, lasheader->laszip->compressor);
        if ((lasheader->laszip->compressor == LASZIP_COMPRESSOR_CHUNKED) || (lasheader->laszip->compressor == LASZIP_COMPRESSOR_LAYERED_CHUNKED)) fprintf(file_out, " %d):", lasheader->laszip->chunk_size);
        else fprintf(file_out, "):");
        for (int i = 0; i < (int)lasheader->laszip->num_items; i++) fprintf(file_out, " %s %d", lasheader->laszip->items[i].get_name(), lasheader->laszip->items[i].version);
        fprintf(file_out, "\012");
    }
    if (lasheader->vlr_lastiling)
    {
        LASquadtree lasquadtree;
        lasquadtree.subtiling_setup(lasheader->vlr_lastiling->min_x, lasheader->vlr_lastiling->max_x, lasheader->vlr_lastiling->min_y, lasheader->vlr_lastiling->max_y, lasheader->vlr_lastiling->level, lasheader->vlr_lastiling->level_index, 0);
        F32 min[2], max[2];
        lasquadtree.get_cell_bounding_box(lasheader->vlr_lastiling->level_index, min, max);
        F32 buffer = 0.0f;
        if (lasheader->vlr_lastiling->buffer)
        {
            buffer = (F32)(min[0] - lasheader->min_x);
            if ((F32)(min[1] - lasheader->min_y) > buffer) buffer = (F32)(min[1] - lasheader->min_y);
            if ((F32)(lasheader->max_x - max[0]) > buffer) buffer = (F32)(lasheader->max_x - max[0]);
            if ((F32)(lasheader->max_y - max[1]) > buffer) buffer = (F32)(lasheader->max_y - max[1]);
        }
        fprintf(file_out, "LAStiling (idx %d, lvl %d, sub %d, bbox %.10g %.10g %.10g %.10g%s%s) (size %g x %g, buffer %g)\n",
            lasheader->vlr_lastiling->level_index,
            lasheader->vlr_lastiling->level,
            lasheader->vlr_lastiling->implicit_levels,
            lasheader->vlr_lastiling->min_x,
            lasheader->vlr_lastiling->min_y,
            lasheader->vlr_lastiling->max_x,
            lasheader->vlr_lastiling->max_y,
            (lasheader->vlr_lastiling->buffer ? ", buffer" : ""),
            (lasheader->vlr_lastiling->reversible ? ", reversible" : ""),
            max[0] - min[0],
            max[1] - min[1],
            buffer);
    }
    if (lasheader->vlr_lasoriginal)
    {
        fprintf(file_out, "LASoriginal (npoints %u, bbox %.10g %.10g %.10g %.10g %.10g %.10g)\n",
            (U32)lasheader->vlr_lasoriginal->number_of_point_records,
            lasheader->vlr_lasoriginal->min_x,
            lasheader->vlr_lasoriginal->min_y,
            lasheader->vlr_lasoriginal->min_z,
            lasheader->vlr_lasoriginal->max_x,
            lasheader->vlr_lasoriginal->max_y,
            lasheader->vlr_lasoriginal->max_z);
    }
}
fclose(file_out);
return 0;
}
int LasToLas::Create(const AppConfig m_AppConfig, int argc, char* argv[], const char* pMetaFileName, const bool isLAS, unsigned int& nEPSG, std::string& strCS, bool &isCreateLAZ,const std::string& geoId,const char* pMetaDestFileName) const
{
    int i;
#ifdef COMPILE_WITH_GUI
    bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
    I32 cores = 1;
    BOOL cpu64 = FALSE;
#endif
    bool error = false;
    bool verbose = false;
    bool very_verbose = false;
    bool force = false;
    // fixed header changes 
    int set_version_major = -1;
    int set_version_minor = -1;
    int set_point_data_format = -1;
    int set_point_data_record_length = -1;
    int set_global_encoding_gps_bit = -1;
    int set_lastiling_buffer_flag = -1;
    // variable header changes
    bool set_ogc_wkt = false;
    bool set_ogc_wkt_in_evlr = false;
    CHAR* set_ogc_wkt_string = 0;
    bool remove_header_padding = false;
    bool remove_all_variable_length_records = false;
    int remove_variable_length_record = -1;
    int remove_variable_length_record_from = -1;
    int remove_variable_length_record_to = -1;
    bool remove_all_extended_variable_length_records = false;
    int remove_extended_variable_length_record = -1;
    int remove_extended_variable_length_record_from = -1;
    int remove_extended_variable_length_record_to = -1;
    CHAR* add_empty_vlr_user_ID = 0;
    int add_empty_vlr_record_ID = -1;
    CHAR* add_empty_vlr_description = 0;
    bool move_evlrs_to_vlrs = false;
    bool save_vlrs = false;
    bool load_vlrs = false;
    int set_attribute_scales = 0;
    int set_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
    double set_attribute_scale_scale[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    int set_attribute_offsets = 0;
    int set_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
    double set_attribute_offset_offset[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    int unset_attribute_scales = 0;
    int unset_attribute_scale_index[5] = { -1, -1, -1, -1, -1 };
    int unset_attribute_offsets = 0;
    int unset_attribute_offset_index[5] = { -1, -1, -1, -1, -1 };
    bool remove_tiling_vlr = false;
    bool remove_original_vlr = false;
    bool remove_empty_files = true;
    // extract a subsequence
    I64 subsequence_start = 0;
    I64 subsequence_stop = I64_MAX;
    // fix files with corrupt points
    bool clip_to_bounding_box = false;
    double start_time = 0;
    
    //destination file info
    std::string tmp ;
    const char* DestFile;

    //crop
    double box[4];

    //proj
    Tranform toWGS84,toTarget;

    GeoidHeight m_GeoidHeight;
    GeoidPROJ m_GeoidPROJ;

    
    char* tempo = argv[0];
    


    if (!m_GeoidHeight.init(m_AppConfig.getPathN2000().c_str(), TYPE_GEOID_N2000))
        fprintf(stderr,"{\"Status\":\"OK\",\"Warning\": \"database caching FIN2005N00.asc not found: " , m_AppConfig.getPathN2000() , "\"}\n");

    if (!m_GeoidHeight.init(m_AppConfig.getPathN60().c_str(), TYPE_GEOID_N60))
        fprintf(stderr,"{\"Status\":\"OK\",\"Warning\": \"database caching FIN2000.asc not found" , m_AppConfig.getPathN60() , "\"}\n");

    if (!m_GeoidPROJ.Create(&m_GeoidHeight, m_AppConfig.getPathConfig(), m_AppConfig.getCatalogPROJ()) )
        fprintf(stderr,"{\"Status\":\"OK\",\"Warning\": \"geoid catalog file geoid.ini not found\"}\n");


    LASreadOpener lasreadopener;
    GeoProjectionConverter geoprojectionconverter;
    LASwriteOpener laswriteopener;

    if (argc == 1)
    {
#ifdef COMPILE_WITH_GUI
        return las2las_gui(argc, argv, 0);
#else
        fprintf(stderr, "%s is better run in the command line\n", argv[0]);
        char file_name[256];
        fprintf(stderr, "enter input file: "); fgets(file_name, 256, stdin);
        file_name[strlen(file_name) - 1] = '\0';
        lasreadopener.set_file_name(file_name);
        fprintf(stderr, "enter output file: "); fgets(file_name, 256, stdin);
        file_name[strlen(file_name) - 1] = '\0';
        laswriteopener.set_file_name(file_name);
#endif
    }
    else
    {
        for (i = 1; i < argc; i++)
        {
            if (argv[i][0] == '\0') continue;
            if (argv[i][0] == '–') argv[i][0] = '-';
            if (strcmp(argv[i], "-week_to_adjusted") == 0)
            {
                set_global_encoding_gps_bit = 1;
            }
            else if (strcmp(argv[i], "-adjusted_to_week") == 0)
            {
                set_global_encoding_gps_bit = 0;
            }
            else if (strcmp(argv[i], "-o") == 0)
            {
                tmp = argv[i + 1];
                DestFile = tmp.c_str();
            }
            else if (strcmp(argv[i], "-epsg") == 0)
            {
                source = atoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "-target_epsg") == 0)
            {
                target = atoi(argv[i + 1]);
                if (target != source) isDest = true;

            }
            else if (strcmp(argv[i], "-source_geoId") == 0)
            {
                *argv[i] = '\0';
                source_geoId = argv[i + 1];
                if (source_geoId != geoId) change_geoId = true;
                *argv[i + 1] = '\0';
            }
            else if (strcmp(argv[i], "-crop") == 0)
            {
                isCrop = true;
                *argv[i] = '\0';
                for (int j = 0; j < 4; j++) {
                    box[j] = atof(argv[i + j + 1]);
                    *argv[i + j + 1] = '\0';
                }
            }
        }

        if (!geoprojectionconverter.parse(argc, argv)) byebye(true);
        if (!lasreadopener.parse(argc, argv)) byebye(true);
        if (!laswriteopener.parse(argc, argv)) byebye(true);
    }
    
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '\0')
        {
            continue;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0)
        {
            fprintf(stderr, "XD LASToLAS version %d\n", LAS_TOOLS_VERSION);
            usage();
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0)
        {
            verbose = true;
        }
        else if (strcmp(argv[i], "-vv") == 0 || strcmp(argv[i], "-very_verbose") == 0)
        {
            very_verbose = true;
        }
        else if (strcmp(argv[i], "-version") == 0)
        {
            fprintf(stderr, "XD LASToLAS version %d\n", LAS_TOOLS_VERSION);
            byebye();
        }
        else if (strcmp(argv[i], "-fail") == 0)
        {
        }
        else if (strcmp(argv[i], "-gui") == 0)
        {
#ifdef COMPILE_WITH_GUI
            gui = true;
#else
            fprintf(stderr, "WARNING: not compiled with GUI support. ignoring '-gui' ...\n");
#endif
        }
        else if (strcmp(argv[i], "-cores") == 0)
        {
#ifdef COMPILE_WITH_MULTI_CORE
            if ((i + 1) >= argc)
            {
                fprintf(stderr, "ERROR: '%s' needs 1 argument: number\n", argv[i]);
                usage(true);
            }
            if (sscanf(argv[i + 1], "%u", &cores) != 1)
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                usage(true);
            }
            argv[i][0] = '\0';
            i++;
            argv[i][0] = '\0';
#else
            fprintf(stderr, "WARNING: not compiled with multi-core batching. ignoring '-cores' ...\n");
            i++;
#endif
        }
        else if (strcmp(argv[i], "-cpu64") == 0)
        {
#ifdef COMPILE_WITH_MULTI_CORE
            cpu64 = TRUE;
#else
            fprintf(stderr, "WARNING: not compiled with 64 bit support. ignoring '-cpu64' ...\n");
#endif
            argv[i][0] = '\0';
        }
        else if (strcmp(argv[i], "-force") == 0)
        {
            force = true;
        }
        else if (strcmp(argv[i], "-subseq") == 0)
        {
            if ((i + 2) >= argc)
            {
                fprintf(stderr, "ERROR: '%s' needs 2 arguments: start stop\n", argv[i]);
                byebye(true);
            }
#ifdef _WIN32
            if (sscanf(argv[i + 1], "%I64d", &subsequence_start) != 1)
#else
            if (sscanf(argv[i + 1], "%lld", &subsequence_start) != 1)
#endif // _WIN32
            {
                fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                usage(true);
            }
#ifdef _WIN32
            if (sscanf(argv[i + 2], "%I64d", &subsequence_stop) != 1)
#else
            if (sscanf(argv[i + 2], "%lld", &subsequence_stop) != 1)
#endif // _WIN32
            {
                fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i + 2], argv[i]);
                usage(true);
            }
            i += 2;
        }
        else if (strcmp(argv[i], "-start_at_point") == 0)
        {
            if ((i + 1) >= argc)
            {
                fprintf(stderr, "ERROR: '%s' needs 1 argument: index of start point\n", argv[i]);
                byebye(true);
            }
#ifdef _WIN32
            if (sscanf(argv[i + 1], "%I64d", &subsequence_start) != 1)
#else
            if (sscanf(argv[i + 1], "%lld", &subsequence_start) != 1)
#endif // _WIN32
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                usage(true);
            }
            i += 1;
        }
        else if (strcmp(argv[i], "-stop_at_point") == 0)
        {
            if ((i + 1) >= argc)
            {
                fprintf(stderr, "ERROR: '%s' needs 1 argument: index of stop point\n", argv[i]);
                byebye(true);
            }
#ifdef _WIN32
            if (sscanf(argv[i + 1], "%I64d", &subsequence_stop) != 1)
#else
            if (sscanf(argv[i + 1], "%lld", &subsequence_stop) != 1)
#endif // _WIN32
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                usage(true);
            }
            i += 1;
        }
        else if (strncmp(argv[i], "-set_", 5) == 0)
        {
            if (strncmp(argv[i], "-set_point_", 11) == 0)
            {
                if (strcmp(argv[i], "-set_point_type") == 0 || strcmp(argv[i], "-set_point_data_format") == 0)
                {
                    if ((i + 1) >= argc)
                    {
                        fprintf(stderr, "ERROR: '%s' needs 1 argument: type\n", argv[i]);
                        byebye(true);
                    }
                    if (sscanf(argv[i + 1], "%u", &set_point_data_format) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    i++;
                }
                else if (strcmp(argv[i], "-set_point_data_record_length") == 0 || strcmp(argv[i], "-set_point_size") == 0)
                {
                    if ((i + 1) >= argc)
                    {
                        fprintf(stderr, "ERROR: '%s' needs 1 argument: size\n", argv[i]);
                        byebye(true);
                    }
                    if (sscanf(argv[i + 1], "%u", &set_point_data_record_length) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    i++;
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                    usage(true);
                }
            }
            else if (strcmp(argv[i], "-set_global_encoding_gps_bit") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
                    byebye(true);
                }
                if (sscanf(argv[i + 1], "%u", &set_global_encoding_gps_bit) != 1)
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                    usage(true);
                }
                i += 1;
            }
            else if (strcmp(argv[i], "-set_version") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: major.minor\n", argv[i]);
                    byebye(true);
                }
                if (sscanf(argv[i + 1], "%u.%u", &set_version_major, &set_version_minor) != 2)
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                    usage(true);
                }
                i += 1;
            }
            else if (strcmp(argv[i], "-set_version_major") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: major\n", argv[i]);
                    byebye(true);
                }
                if (sscanf(argv[i + 1], "%u", &set_version_major) != 1)
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                    usage(true);
                }
                i += 1;
            }
            else if (strcmp(argv[i], "-set_version_minor") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: minor\n", argv[i]);
                    byebye(true);
                }
                if (sscanf(argv[i + 1], "%u", &set_version_minor) != 1)
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                    usage(true);
                }
                i += 1;
            }
            else if (strcmp(argv[i], "-set_lastiling_buffer_flag") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
                    byebye(true);
                }
                if (sscanf(argv[i + 1], "%u", &set_lastiling_buffer_flag) != 1)
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                    usage(true);
                }
                if (set_lastiling_buffer_flag > 1)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: 0 or 1\n", argv[i]);
                    byebye(true);
                }
                i += 1;
            }
            else if (strncmp(argv[i], "-set_ogc_wkt", 12) == 0)
            {
                if (strcmp(argv[i], "-set_ogc_wkt") == 0)
                {
                    set_ogc_wkt = true;
                    set_ogc_wkt_in_evlr = false;
                }
                else if (strcmp(argv[i], "-set_ogc_wkt_in_evlr") == 0)
                {
                    set_ogc_wkt = true;
                    set_ogc_wkt_in_evlr = true;
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                    usage(true);
                }
                if ((i + 1) < argc)
                {
                    if ((argv[i + 1][0] != '-') && (argv[i + 1][0] != '\0'))
                    {
                        set_ogc_wkt_string = argv[i + 1];
                        i++;
                    }
                }
            }
            else if (strcmp(argv[i], "-set_attribute_scale") == 0)
            {
                if ((i + 2) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 2 arguments: index scale\n", argv[i]);
                    byebye(true);
                }
                if (set_attribute_scales < 5)
                {
                    if (sscanf(argv[i + 1], "%u", &(set_attribute_scale_index[set_attribute_scales])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    if (sscanf(argv[i + 2], "%lf", &(set_attribute_scale_scale[set_attribute_scales])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i + 2], argv[i]);
                        usage(true);
                    }
                    set_attribute_scales++;
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot '%s' more than 5 times\n", argv[i]);
                    byebye(true);
                }
                i += 2;
            }
            else if (strcmp(argv[i], "-set_attribute_offset") == 0)
            {
                if ((i + 2) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 2 arguments: index offset\n", argv[i]);
                    byebye(true);
                }
                if (set_attribute_offsets < 5)
                {
                    if (sscanf(argv[i + 1], "%u", &(set_attribute_offset_index[set_attribute_offsets])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand first argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    if (sscanf(argv[i + 2], "%lf", &(set_attribute_offset_offset[set_attribute_offsets])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand second argument '%s' for '%s'\n", argv[i + 2], argv[i]);
                        usage(true);
                    }
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot '%s' more than 5 times\n", argv[i]);
                    byebye(true);
                }
                i += 2;
            }
            else
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                usage(true);
            }
        }
        else if (strncmp(argv[i], "-remove_", 8) == 0)
        {
            if (strcmp(argv[i], "-remove_padding") == 0)
            {
                remove_header_padding = true;
            }
            else if (strcmp(argv[i], "-remove_all_vlrs") == 0)
            {
                remove_all_variable_length_records = true;
            }
            else if (strcmp(argv[i], "-remove_vlr") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: number\n", argv[i]);
                    byebye(true);
                }
                remove_variable_length_record = atoi(argv[i + 1]);
                remove_variable_length_record_from = -1;
                remove_variable_length_record_to = -1;
                i++;
            }
            else if (strcmp(argv[i], "-remove_vlrs_from_to") == 0)
            {
                if ((i + 2) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 2 arguments: start end\n", argv[i]);
                    byebye(true);
                }
                remove_variable_length_record = -1;
                remove_variable_length_record_from = atoi(argv[i + 1]);
                remove_variable_length_record_to = atoi(argv[i + 2]);
                i += 2;
            }
            else if (strcmp(argv[i], "-remove_all_evlrs") == 0)
            {
                remove_all_extended_variable_length_records = true;
            }
            else if (strcmp(argv[i], "-remove_evlr") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: number\n", argv[i]);
                    byebye(true);
                }
                remove_extended_variable_length_record = atoi(argv[i + 1]);
                remove_extended_variable_length_record_from = -1;
                remove_extended_variable_length_record_to = -1;
                i++;
            }
            else if (strcmp(argv[i], "-remove_evlrs_from_to") == 0)
            {
                if ((i + 2) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 2 arguments: start end\n", argv[i]);
                    byebye(true);
                }
                remove_extended_variable_length_record = -1;
                remove_extended_variable_length_record_from = atoi(argv[i + 1]);
                remove_extended_variable_length_record_to = atoi(argv[i + 2]);
                i += 2;
            }
            else if (strcmp(argv[i], "-remove_tiling_vlr") == 0)
            {
                remove_tiling_vlr = true;
            }
            else if (strcmp(argv[i], "-remove_original_vlr") == 0)
            {
                remove_original_vlr = true;
            }
            else
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                usage(true);
            }
        }
        else if (strncmp(argv[i], "-add_", 5) == 0)
        {
            if (strcmp(argv[i], "-add_attribute") == 0)
            {
                if ((i + 3) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs at least 3 arguments: data_type name description\n", argv[i]);
                    usage(true);
                }
                if (((i + 4) < argc) && (atof(argv[i + 4]) != 0.0))
                {
                    if (((i + 5) < argc) && ((atof(argv[i + 5]) != 0.0) || (strcmp(argv[i + 5], "0") == 0) || (strcmp(argv[i + 5], "0.0") == 0)))
                    {
                        if (((i + 6) < argc) && ((atof(argv[i + 6]) != 0.0) || (strcmp(argv[i + 6], "0") == 0) || (strcmp(argv[i + 6], "0.0") == 0)))
                        {
                            lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]), 1.0, 0.0, atof(argv[i + 6]));
                            i += 6;
                        }
                        else
                        {
                            lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]), atof(argv[i + 5]));
                            i += 5;
                        }
                    }
                    else
                    {
                        lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3], atof(argv[i + 4]));
                        i += 4;
                    }
                }
                else
                {
                    lasreadopener.add_attribute(atoi(argv[i + 1]), argv[i + 2], argv[i + 3]);
                    i += 3;
                }
            }
            else if (strcmp(argv[i], "-add_empty_vlr") == 0)
            {
                if ((i + 2) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs at least 2 arguments: user_ID and record_ID\n", argv[i]);
                    usage(true);
                }
                add_empty_vlr_user_ID = argv[i + 1];
                add_empty_vlr_record_ID = atoi(argv[i + 2]);
                i += 2;
                if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
                {
                    add_empty_vlr_description = argv[i + 1];
                    i += 1;
                }
            }
            else
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                usage(true);
            }
        }
        else if (strncmp(argv[i], "-unset_", 7) == 0)
        {
            if (strcmp(argv[i], "-unset_attribute_scale") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: index\n", argv[i]);
                    byebye(true);
                }
                if (unset_attribute_scales < 5)
                {
                    if (sscanf(argv[i + 1], "%u", &(unset_attribute_scale_index[unset_attribute_scales])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    unset_attribute_scales++;
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot '%s' more than 5 times\n", argv[i]);
                    byebye(true);
                }
                i += 1;
            }
            else if (strcmp(argv[i], "-unset_attribute_offset") == 0)
            {
                if ((i + 1) >= argc)
                {
                    fprintf(stderr, "ERROR: '%s' needs 1 argument: index\n", argv[i]);
                    byebye(true);
                }
                if (unset_attribute_offsets < 5)
                {
                    if (sscanf(argv[i + 1], "%u", &(unset_attribute_offset_index[unset_attribute_offsets])) != 1)
                    {
                        fprintf(stderr, "ERROR: cannot understand argument '%s' for '%s'\n", argv[i + 1], argv[i]);
                        usage(true);
                    }
                    unset_attribute_offsets++;
                }
                else
                {
                    fprintf(stderr, "ERROR: cannot '%s' more than 5 times\n", argv[i]);
                    byebye(true);
                }
                i += 1;
            }
            else
            {
                fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
                usage(true);
            }
        }
        else if (strcmp(argv[i], "-move_evlrs_to_vlrs") == 0)
        {
            move_evlrs_to_vlrs = true;
        }
        else if (strcmp(argv[i], "-save_vlrs") == 0)
        {
            save_vlrs = true;
        }
        else if (strcmp(argv[i], "-load_vlrs") == 0)
        {
            load_vlrs = true;
        }
        else if (strcmp(argv[i], "-dont_remove_empty_files") == 0)
        {
            remove_empty_files = false;
        }
        else if (strcmp(argv[i], "-clip_to_bounding_box") == 0 || strcmp(argv[i], "-clip_to_bb") == 0)
        {
            clip_to_bounding_box = true;
        }
        else if ((argv[i][0] != '-') && (lasreadopener.get_file_name_number() == 0))
        {
            lasreadopener.add_file_name(argv[i]);
            argv[i][0] = '\0';
        }
        else
        {
            fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
            usage(true);
        }
    }

#ifdef COMPILE_WITH_GUI
    if (gui)
    {
        return las2las_gui(argc, argv, &lasreadopener);
    }
#endif

#ifdef COMPILE_WITH_MULTI_CORE
    if (cores > 1)
    {
        if (lasreadopener.get_file_name_number() < 2)
        {
            fprintf(stderr, "WARNING: only %u input files. ignoring '-cores %d' ...\n", lasreadopener.get_file_name_number(), cores);
        }
        else if (lasreadopener.is_merged())
        {
            fprintf(stderr, "WARNING: input files merged on-the-fly. ignoring '-cores %d' ...\n", cores);
        }
        else
        {
            return las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, cores, cpu64);
        }
    }
    if (cpu64)
    {
        return las2las_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
    }
#endif

    // check input
    if (!lasreadopener.active())
    {
        fprintf(stderr, "ERROR: no input specified\n");
        usage(true, argc == 1);
    }

    BOOL extra_pass = laswriteopener.is_piped();

    // we only really need an extra pass if the coordinates are altered or if points are filtered

    if (extra_pass)
    {
        if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
        {
            extra_pass = FALSE;
        }
    }

    // for piped output we need an extra pass

    if (extra_pass)
    {
        if (lasreadopener.is_piped())
        {
            fprintf(stderr, "ERROR: input and output cannot both be piped\n");
            usage(true);
        }
    }

    // only save or load

    if (save_vlrs && load_vlrs)
    {
        fprintf(stderr, "ERROR: cannot save and load VLRs at the same time\n");
        usage(true);
    }

    // possibly loop over multiple input files

    while (lasreadopener.active())
    {
        try
        {
            if (verbose) start_time = taketime();

            // open lasreader

            LASreader* lasreader = lasreadopener.open();

            if (lasreader == 0)
            {
                fprintf(stderr, "ERROR: could not open lasreader\n");
                usage(true, argc == 1);
            }
            isCreateLAZ = true;
            // save to metadata
            if (pMetaFileName)
            {
                getInfor(lasreader, &geoprojectionconverter, pMetaFileName, nEPSG, strCS);
                if (!isLAS && (nEPSG > 0 || !strCS.empty()) && !isDest && !isCrop )
                {
                    // don't change EPSG with LAZ file
                    lasreader->close();
                    delete lasreader;
                    isCreateLAZ = false;
                    continue;
                }
                if (nEPSG > 0) {
                    int Newargc = 5;
                    char* Newargv[] = { tempo,(char*)string("-epsg").c_str(),(char*)to_string(nEPSG).c_str(),(char*) string("-target_epsg").c_str(),(char*)to_string(target).c_str()};
                    source = nEPSG;
                    if (!geoprojectionconverter.parse(Newargc, Newargv)) byebye(true);
                }
            }
                
            // store the inventory for the header

            LASinventory lasinventory;

            // the point we write sometimes needs to be copied

            LASpoint* point = 0;

            // prepare the header for output

            if (set_global_encoding_gps_bit != -1)
            {
                if (set_global_encoding_gps_bit == 0)
                {
                    if ((lasreader->header.global_encoding & 1) == 0)
                    {
                        fprintf(stderr, "WARNING: global encoding indicates file already in GPS week time\n");
                        if (force)
                        {
                            fprintf(stderr, "         forced conversion.\n");
                        }
                        else
                        {
                            fprintf(stderr, "         use '-force' to force conversion.\n");
                            byebye(true);
                        }
                    }
                    else
                    {
                        lasreader->header.global_encoding &= ~1;
                    }
                }
                else if (set_global_encoding_gps_bit == 1)
                {
                    if ((lasreader->header.global_encoding & 1) == 1)
                    {
                        fprintf(stderr, "WARNING: global encoding indicates file already in Adjusted Standard GPS time\n");
                        if (force)
                        {
                            fprintf(stderr, "         forced conversion.\n");
                        }
                        else
                        {
                            fprintf(stderr, "         use '-force' to force conversion.\n");
                            byebye(true);
                        }
                    }
                    else
                    {
                        lasreader->header.global_encoding |= 1;
                    }
                }
                else
                {
                    fprintf(stderr, "WARNING: ignoring invalid option '-set_global_encoding_gps_bit %d'\n", set_global_encoding_gps_bit);
                }
            }

            if (set_attribute_scales)
            {
                for (i = 0; i < set_attribute_scales; i++)
                {
                    if (set_attribute_scale_index[i] != -1)
                    {
                        if (set_attribute_scale_index[i] < lasreader->header.number_attributes)
                        {
                            lasreader->header.attributes[set_attribute_scale_index[i]].set_scale(set_attribute_scale_scale[i]);
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", set_attribute_scale_index[i], lasreader->header.number_attributes);
                        }
                    }
                }
            }

            if (set_attribute_offsets)
            {
                for (i = 0; i < set_attribute_offsets; i++)
                {
                    if (set_attribute_offset_index[i] != -1)
                    {
                        if (set_attribute_offset_index[i] < lasreader->header.number_attributes)
                        {
                            lasreader->header.attributes[set_attribute_offset_index[i]].set_offset(set_attribute_offset_offset[i]);
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", set_attribute_offset_index[i], lasreader->header.number_attributes);
                        }
                    }
                }
            }

            if (unset_attribute_scales)
            {
                for (i = 0; i < unset_attribute_scales; i++)
                {
                    if (unset_attribute_scale_index[i] != -1)
                    {
                        if (unset_attribute_scale_index[i] < lasreader->header.number_attributes)
                        {
                            lasreader->header.attributes[unset_attribute_scale_index[i]].unset_scale();
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", unset_attribute_scale_index[i], lasreader->header.number_attributes);
                        }
                    }
                }
            }

            if (unset_attribute_offsets)
            {
                for (i = 0; i < unset_attribute_offsets; i++)
                {
                    if (unset_attribute_offset_index[i] != -1)
                    {
                        if (unset_attribute_offset_index[i] < lasreader->header.number_attributes)
                        {
                            lasreader->header.attributes[unset_attribute_offset_index[i]].unset_offset();
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: attribute index %d out-of-range. only %d attributes in file. ignoring ... \n", unset_attribute_offset_index[i], lasreader->header.number_attributes);
                        }
                    }
                }
            }

            if (set_attribute_scales || set_attribute_offsets || unset_attribute_scales || unset_attribute_offsets)
            {
                lasreader->header.update_extra_bytes_vlr();
            }

            if (set_point_data_format > 5)
            {
                if (set_version_minor == -1)
                {
                    set_version_minor = 4;
                }
            }

            if (set_version_major != -1)
            {
                if (set_version_major != 1)
                {
                    fprintf(stderr, "ERROR: unknown version_major %d\n", set_version_major);
                    byebye(true);
                }
                lasreader->header.version_major = (U8)set_version_major;
            }

            if (set_version_minor >= 0)
            {
                if (set_version_minor > 4)
                {
                    fprintf(stderr, "ERROR: unknown version_minor %d\n", set_version_minor);
                    byebye(true);
                }
                if (set_version_minor < 3)
                {
                    if (lasreader->header.version_minor == 3)
                    {
                        lasreader->header.header_size -= 8;
                        lasreader->header.offset_to_point_data -= 8;
                    }
                    else if (lasreader->header.version_minor >= 4)
                    {
                        lasreader->header.header_size -= (8 + 140);
                        lasreader->header.offset_to_point_data -= (8 + 140);
                    }
                }
                else if (set_version_minor == 3)
                {
                    if (lasreader->header.version_minor < 3)
                    {
                        lasreader->header.header_size += 8;
                        lasreader->header.offset_to_point_data += 8;
                        lasreader->header.start_of_waveform_data_packet_record = 0;
                    }
                    else if (lasreader->header.version_minor >= 4)
                    {
                        lasreader->header.header_size -= 140;
                        lasreader->header.offset_to_point_data -= 140;
                    }
                }
                else if (set_version_minor == 4)
                {
                    if (lasreader->header.version_minor < 3)
                    {
                        lasreader->header.header_size += (8 + 140);
                        lasreader->header.offset_to_point_data += (8 + 140);
                        lasreader->header.start_of_waveform_data_packet_record = 0;
                    }
                    else if (lasreader->header.version_minor == 3)
                    {
                        lasreader->header.header_size += 140;
                        lasreader->header.offset_to_point_data += 140;
                    }

                    if (lasreader->header.version_minor < 4)
                    {
                        if (set_point_data_format > 5)
                        {
                            lasreader->header.extended_number_of_point_records = lasreader->header.number_of_point_records;
                            lasreader->header.number_of_point_records = 0;
                            for (i = 0; i < 5; i++)
                            {
                                lasreader->header.extended_number_of_points_by_return[i] = lasreader->header.number_of_points_by_return[i];
                                lasreader->header.number_of_points_by_return[i] = 0;
                            }
                        }
                    }
                }

                if ((set_version_minor <= 3) && (lasreader->header.version_minor >= 4))
                {
                    if (lasreader->header.point_data_format > 5)
                    {
                        switch (lasreader->header.point_data_format)
                        {
                        case 6:
                            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 1\n", lasreader->header.point_data_format);
                            lasreader->header.point_data_format = 1;
                            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
                            lasreader->header.point_data_record_length -= 2;
                            break;
                        case 7:
                            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 3\n", lasreader->header.point_data_format);
                            lasreader->header.point_data_format = 3;
                            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
                            lasreader->header.point_data_record_length -= 2;
                            break;
                        case 8:
                            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 3\n", lasreader->header.point_data_format);
                            lasreader->header.point_data_format = 3;
                            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
                            lasreader->header.point_data_record_length -= 4;
                            break;
                        case 9:
                            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 4\n", lasreader->header.point_data_format);
                            lasreader->header.point_data_format = 4;
                            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 2);
                            lasreader->header.point_data_record_length -= 2;
                            break;
                        case 10:
                            fprintf(stderr, "WARNING: downgrading point_data_format from %d to 5\n", lasreader->header.point_data_format);
                            lasreader->header.point_data_format = 5;
                            fprintf(stderr, "         and point_data_record_length from %d to %d\n", lasreader->header.point_data_record_length, lasreader->header.point_data_record_length - 4);
                            lasreader->header.point_data_record_length -= 4;
                            break;
                        default:
                            fprintf(stderr, "ERROR: unknown point_data_format %d\n", lasreader->header.point_data_format);
                            byebye(true);
                        }
                        point = new LASpoint;
                        lasreader->header.clean_laszip();
                    }
                    if (lasreader->header.get_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS))
                    {
                        fprintf(stderr, "WARNING: unsetting global encoding bit %d when downgrading from version 1.%d to version 1.%d\n", LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS, lasreader->header.version_minor, set_version_minor);
                        lasreader->header.unset_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
                    }
                    if (lasreader->header.number_of_extended_variable_length_records)
                    {
                        fprintf(stderr, "WARNING: loosing %d EVLR%s when downgrading from version 1.%d to version 1.%d\n         attempting to move %s to the VLR section ...\n", lasreader->header.number_of_extended_variable_length_records, (lasreader->header.number_of_extended_variable_length_records > 1 ? "s" : ""), lasreader->header.version_minor, set_version_minor, (lasreader->header.number_of_extended_variable_length_records > 1 ? "them" : "it"));

                        U32 u;
                        for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
                        {
                            if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
                            {
                                lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
                                lasreader->header.evlrs[u].data = 0;
#ifdef _WIN32
                                fprintf(stderr, "         moved EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
                                fprintf(stderr, "         moved EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
                            }
                            else
                            {
#ifdef _WIN32
                                fprintf(stderr, "         lost EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
                                fprintf(stderr, "         lost EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
                            }
                        }
                    }
                }

                lasreader->header.version_minor = (U8)set_version_minor;
            }

            // are we supposed to change the point data format

            if (set_point_data_format != -1)
            {
                if (set_point_data_format < 0 || set_point_data_format > 10)
                {
                    fprintf(stderr, "ERROR: unknown point_data_format %d\n", set_point_data_format);
                    byebye(true);
                }
                // depending on the conversion we may need to copy the point
                if (convert_point_type_from_to[lasreader->header.point_data_format][set_point_data_format])
                {
                    if (point == 0) point = new LASpoint;
                }
                // were there extra bytes before
                I32 num_extra_bytes = 0;
                switch (lasreader->header.point_data_format)
                {
                case 0:
                    num_extra_bytes = lasreader->header.point_data_record_length - 20;
                    break;
                case 1:
                    num_extra_bytes = lasreader->header.point_data_record_length - 28;
                    break;
                case 2:
                    num_extra_bytes = lasreader->header.point_data_record_length - 26;
                    break;
                case 3:
                    num_extra_bytes = lasreader->header.point_data_record_length - 34;
                    break;
                case 4:
                    num_extra_bytes = lasreader->header.point_data_record_length - 57;
                    break;
                case 5:
                    num_extra_bytes = lasreader->header.point_data_record_length - 63;
                    break;
                case 6:
                    num_extra_bytes = lasreader->header.point_data_record_length - 30;
                    break;
                case 7:
                    num_extra_bytes = lasreader->header.point_data_record_length - 36;
                    break;
                case 8:
                    num_extra_bytes = lasreader->header.point_data_record_length - 38;
                    break;
                case 9:
                    num_extra_bytes = lasreader->header.point_data_record_length - 59;
                    break;
                case 10:
                    num_extra_bytes = lasreader->header.point_data_record_length - 67;
                    break;
                }
                if (num_extra_bytes < 0)
                {
                    fprintf(stderr, "ERROR: point record length has %d fewer bytes than needed\n", -num_extra_bytes);
                    byebye(true);
                }
                lasreader->header.point_data_format = (U8)set_point_data_format;
                lasreader->header.clean_laszip();
                switch (lasreader->header.point_data_format)
                {
                case 0:
                    lasreader->header.point_data_record_length = 20 + num_extra_bytes;
                    break;
                case 1:
                    lasreader->header.point_data_record_length = 28 + num_extra_bytes;
                    break;
                case 2:
                    lasreader->header.point_data_record_length = 26 + num_extra_bytes;
                    break;
                case 3:
                    lasreader->header.point_data_record_length = 34 + num_extra_bytes;
                    break;
                case 4:
                    lasreader->header.point_data_record_length = 57 + num_extra_bytes;
                    break;
                case 5:
                    lasreader->header.point_data_record_length = 63 + num_extra_bytes;
                    break;
                case 6:
                    lasreader->header.point_data_record_length = 30 + num_extra_bytes;
                    break;
                case 7:
                    lasreader->header.point_data_record_length = 36 + num_extra_bytes;
                    break;
                case 8:
                    lasreader->header.point_data_record_length = 38 + num_extra_bytes;
                    break;
                case 9:
                    lasreader->header.point_data_record_length = 59 + num_extra_bytes;
                    break;
                case 10:
                    lasreader->header.point_data_record_length = 67 + num_extra_bytes;
                    break;
                }
            }

            // are we supposed to change the point data record length

            if (set_point_data_record_length != -1)
            {
                I32 num_extra_bytes = 0;
                switch (lasreader->header.point_data_format)
                {
                case 0:
                    num_extra_bytes = set_point_data_record_length - 20;
                    break;
                case 1:
                    num_extra_bytes = set_point_data_record_length - 28;
                    break;
                case 2:
                    num_extra_bytes = set_point_data_record_length - 26;
                    break;
                case 3:
                    num_extra_bytes = set_point_data_record_length - 34;
                    break;
                case 4:
                    num_extra_bytes = set_point_data_record_length - 57;
                    break;
                case 5:
                    num_extra_bytes = set_point_data_record_length - 63;
                    break;
                case 6:
                    num_extra_bytes = set_point_data_record_length - 30;
                    break;
                case 7:
                    num_extra_bytes = set_point_data_record_length - 36;
                    break;
                case 8:
                    num_extra_bytes = set_point_data_record_length - 38;
                    break;
                case 9:
                    num_extra_bytes = set_point_data_record_length - 59;
                    break;
                case 10:
                    num_extra_bytes = set_point_data_record_length - 67;
                    break;
                }
                if (num_extra_bytes < 0)
                {
                    fprintf(stderr, "ERROR: point_data_format %d needs record length of at least %d\n", lasreader->header.point_data_format, set_point_data_record_length - num_extra_bytes);
                    byebye(true);
                }
                if (lasreader->header.point_data_record_length < set_point_data_record_length)
                {
                    if (!point) point = new LASpoint;
                }
                lasreader->header.point_data_record_length = (U16)set_point_data_record_length;
                lasreader->header.clean_laszip();
            }

            // are we supposed to add attributes

            if (lasreadopener.get_number_attributes())
            {
                I32 attibutes_before_size = lasreader->header.get_attributes_size();
                for (i = 0; i < lasreadopener.get_number_attributes(); i++)
                {
                    I32 type = (lasreadopener.get_attribute_data_type(i) - 1) % 10;
                    try {
                        LASattribute attribute(type, lasreadopener.get_attribute_name(i), lasreadopener.get_attribute_description(i));
                        if (lasreadopener.get_attribute_scale(i) != 1.0 || lasreadopener.get_attribute_offset(i) != 0.0)
                        {
                            attribute.set_scale(lasreadopener.get_attribute_scale(i));
                        }
                        if (lasreadopener.get_attribute_offset(i) != 0.0)
                        {
                            attribute.set_offset(lasreadopener.get_attribute_offset(i));
                        }
                        if (lasreadopener.get_attribute_no_data(i) != F64_MAX)
                        {
                            attribute.set_no_data(lasreadopener.get_attribute_no_data(i));
                        }
                        lasreader->header.add_attribute(attribute);
                    }
                    catch (...) {
                        fprintf(stderr, "ERROR: initializing attribute %s\n", lasreadopener.get_attribute_name(i));
                        byebye(true);
                    }
                }
                I32 attibutes_after_size = lasreader->header.get_attributes_size();
                if (!point) point = new LASpoint;
                lasreader->header.update_extra_bytes_vlr();
                lasreader->header.point_data_record_length += (attibutes_after_size - attibutes_before_size);
                lasreader->header.clean_laszip();
            }

            if (set_lastiling_buffer_flag != -1)
            {
                if (lasreader->header.vlr_lastiling)
                {
                    lasreader->header.vlr_lastiling->buffer = set_lastiling_buffer_flag;
                }
                else
                {
                    fprintf(stderr, "WARNING: file '%s' has no LAStiling VLR. cannot set buffer flag.\n", lasreadopener.get_file_name());
                }
            }

            if (move_evlrs_to_vlrs)
            {
                if (lasreader->header.number_of_extended_variable_length_records > 0)
                {
                    U32 u;
                    for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
                    {
                        if (lasreader->header.evlrs[u].record_length_after_header <= U16_MAX)
                        {
                            lasreader->header.add_vlr(lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_id, (U16)lasreader->header.evlrs[u].record_length_after_header, lasreader->header.evlrs[u].data);
                            lasreader->header.evlrs[u].data = 0;
#ifdef _WIN32
                            if (very_verbose) fprintf(stderr, "         moved EVLR %d with user ID '%s' and %I64d bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#else
                            if (very_verbose) fprintf(stderr, "         moved EVLR %d with user ID '%s' and %lld bytes of payload\n", u, lasreader->header.evlrs[u].user_id, lasreader->header.evlrs[u].record_length_after_header);
#endif
                        }
                    }
                    U32 remaining = 0;
                    for (u = 0; u < lasreader->header.number_of_extended_variable_length_records; u++)
                    {
                        if (lasreader->header.evlrs[u].record_length_after_header > U16_MAX)
                        {
                            lasreader->header.evlrs[remaining] = lasreader->header.evlrs[u];
                            remaining++;
                        }
                    }
                    if (verbose) fprintf(stderr, "moved %u EVLRs to VLRs. %u EVLRs with large payload remain.\n", u - remaining, remaining);
                    lasreader->header.number_of_extended_variable_length_records = remaining;
                }
            }

            // if the point needs to be copied set up the data fields

            if (point)
            {
                point->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);
            }

            // reproject or just set the projection?
            LASquantizer* reproject_quantizer = 0;
            LASquantizer* saved_quantizer = 0;
            bool set_projection_in_header = false;

            if (geoprojectionconverter.has_projection(false)) // reproject because a target projection was provided in the command line
            {
                if (!geoprojectionconverter.has_projection(true))      // if no source projection was provided in the command line ...
                {
                    if (lasreader->header.vlr_geo_ogc_wkt)               // ... try to get it from the OGC WKT string ...
                    {
                        geoprojectionconverter.set_projection_from_ogc_wkt(lasreader->header.vlr_geo_ogc_wkt);
                    }
                    if (!geoprojectionconverter.has_projection(true))    // ... nothing ... ? ...
                    {
                        if (lasreader->header.vlr_geo_keys)                // ... try to get it from the geo keys.
                        {
                            geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
                        }
                    }
                }
                if (!geoprojectionconverter.has_projection(true))
                {
                    fprintf(stderr, "WARNING: cannot determine source projection of '%s'. not reprojecting ... \n", lasreadopener.get_file_name());

                    set_projection_in_header = false;
                }
                else
                {
                    geoprojectionconverter.check_horizontal_datum_before_reprojection();

                    reproject_quantizer = new LASquantizer();
                    double point[3];
                    point[0] = (lasreader->header.min_x + lasreader->header.max_x) / 2;
                    point[1] = (lasreader->header.min_y + lasreader->header.max_y) / 2;
                    point[2] = (lasreader->header.min_z + lasreader->header.max_z) / 2;
                    geoprojectionconverter.to_target(point);
                    reproject_quantizer->x_scale_factor = geoprojectionconverter.get_target_precision();
                    reproject_quantizer->y_scale_factor = geoprojectionconverter.get_target_precision();
                    reproject_quantizer->z_scale_factor = (geoprojectionconverter.has_target_elevation_precision() ? geoprojectionconverter.get_target_elevation_precision() : lasreader->header.z_scale_factor);
                    reproject_quantizer->x_offset = ((I64)((point[0] / reproject_quantizer->x_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->x_scale_factor;
                    reproject_quantizer->y_offset = ((I64)((point[1] / reproject_quantizer->y_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->y_scale_factor;
                    //if (target == 4978)
                      //  reproject_quantizer->z_offset = 0;
                    //else
                        reproject_quantizer->z_offset = ((I64)((point[2] / reproject_quantizer->z_scale_factor) / 10000000)) * 10000000 * reproject_quantizer->z_scale_factor;           

                    set_projection_in_header = true;
                }
            }
            else if (geoprojectionconverter.has_projection(true)) // set because only a source projection was provided in the command line
            {
                set_projection_in_header = true;
            }

            if (set_projection_in_header)
            {
                int number_of_keys;
                GeoProjectionGeoKeys* geo_keys = 0;
                int num_geo_double_params;
                double* geo_double_params = 0;

                if (geoprojectionconverter.get_geo_keys_from_projection(number_of_keys, &geo_keys, num_geo_double_params, &geo_double_params, !geoprojectionconverter.has_projection(false)))
                {
                    lasreader->header.set_geo_keys(number_of_keys, (LASvlr_key_entry*)geo_keys);
                    free(geo_keys);
                    if (geo_double_params)
                    {
                        lasreader->header.set_geo_double_params(num_geo_double_params, geo_double_params);
                        free(geo_double_params);
                    }
                    else
                    {
                        lasreader->header.del_geo_double_params();
                    }
                    lasreader->header.del_geo_ascii_params();
                    lasreader->header.del_geo_ogc_wkt();
                }

                if (set_ogc_wkt || (lasreader->header.point_data_format >= 6)) // maybe also set the OCG WKT
                {
                    CHAR* ogc_wkt = set_ogc_wkt_string;
                    I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);
                    if (ogc_wkt == 0)
                    {
                        if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt, !geoprojectionconverter.has_projection(false)))
                        {
                            fprintf(stderr, "WARNING: cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
                            if (ogc_wkt) free(ogc_wkt);
                            ogc_wkt = 0;
                        }
                    }
                    if (ogc_wkt)
                    {
                        if (set_ogc_wkt_in_evlr)
                        {
                            if (lasreader->header.version_minor >= 4)
                            {
                                lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
                            }
                            else
                            {
                                fprintf(stderr, "WARNING: input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...\n", lasreader->header.version_minor);
                                lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
                            }
                        }
                        else
                        {
                            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt);
                        }
                        if (!set_ogc_wkt_string) free(ogc_wkt);
                        if ((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6))
                        {
                            lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
                        }
                    }
                }
            }
            else if (set_ogc_wkt) // maybe only set the OCG WKT 
            {
                CHAR* ogc_wkt = set_ogc_wkt_string;
                I32 len = (ogc_wkt ? (I32)strlen(ogc_wkt) : 0);

                if (ogc_wkt == 0)
                {
                    if (lasreader->header.vlr_geo_keys)
                    {
                        geoprojectionconverter.set_projection_from_geo_keys(lasreader->header.vlr_geo_keys[0].number_of_keys, (GeoProjectionGeoKeys*)lasreader->header.vlr_geo_key_entries, lasreader->header.vlr_geo_ascii_params, lasreader->header.vlr_geo_double_params);
                        if (!geoprojectionconverter.get_ogc_wkt_from_projection(len, &ogc_wkt))
                        {
                            fprintf(stderr, "WARNING: cannot produce OCG WKT. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
                            if (ogc_wkt) free(ogc_wkt);
                            ogc_wkt = 0;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "WARNING: no projection information. ignoring '-set_ogc_wkt' for '%s'\n", lasreadopener.get_file_name());
                    }
                }

                if (ogc_wkt)
                {
                    if (set_ogc_wkt_in_evlr)
                    {
                        if (lasreader->header.version_minor >= 4)
                        {
                            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, TRUE);
                        }
                        else
                        {
                            fprintf(stderr, "WARNING: input file is LAS 1.%d. setting OGC WKT to VLR instead of EVLR ...\n", lasreader->header.version_minor);
                            lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
                        }
                    }
                    else
                    {
                        lasreader->header.set_geo_ogc_wkt(len, ogc_wkt, FALSE);
                    }

                    if (!set_ogc_wkt_string) free(ogc_wkt);

                    if ((lasreader->header.version_minor >= 4) && (lasreader->header.point_data_format >= 6))
                    {
                        lasreader->header.set_global_encoding_bit(LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS);
                    }
                }
            }

            // maybe we should remove some stuff

            if (remove_header_padding)
            {
                lasreader->header.clean_user_data_in_header();
                lasreader->header.clean_user_data_after_header();
            }

            if (remove_all_variable_length_records)
            {
                lasreader->header.clean_vlrs();
            }
            else
            {
                if (remove_variable_length_record != -1)
                {
                    lasreader->header.remove_vlr(remove_variable_length_record);
                }

                if (remove_variable_length_record_from != -1)
                {
                    for (i = remove_variable_length_record_to; i >= remove_variable_length_record_from; i--)
                    {
                        lasreader->header.remove_vlr(i);
                    }
                }
            }

            if (remove_all_extended_variable_length_records)
            {
                lasreader->header.clean_evlrs();
            }
            else
            {
                if (remove_extended_variable_length_record != -1)
                {
                    lasreader->header.remove_evlr(remove_extended_variable_length_record);
                }

                if (remove_extended_variable_length_record_from != -1)
                {
                    for (i = remove_extended_variable_length_record_to; i >= remove_extended_variable_length_record_from; i--)
                    {
                        lasreader->header.remove_evlr(i);
                    }
                }
            }

            if (remove_tiling_vlr)
            {
                lasreader->header.clean_lastiling();
            }

            if (remove_original_vlr)
            {
                lasreader->header.clean_lasoriginal();
            }

            if (lasreader->header.vlr_lastiling || lasreader->header.vlr_lasoriginal)
            {
                if (lasreader->get_transform())
                {
                    if (lasreader->get_transform()->transformed_fields & (LASTRANSFORM_X_COORDINATE | LASTRANSFORM_Y_COORDINATE))
                    {
                        fprintf(stderr, "WARNING: transforming x or y coordinates of file with %s VLR invalidates this VLR\n", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
                    }
                }
                if (reproject_quantizer)
                {
                    fprintf(stderr, "WARNING: reprojecting file with %s VLR invalidates this VLR\n", (lasreader->header.vlr_lastiling ? "lastiling" : "lasoriginal"));
                }
            }

            if (save_vlrs)
            {
                save_vlrs_to_file(&lasreader->header);
                save_vlrs = false;
            }

            if (load_vlrs)
            {
                load_vlrs_from_file(&lasreader->header);
            }

            if (add_empty_vlr_user_ID != 0)
            {
                lasreader->header.add_vlr(add_empty_vlr_user_ID, add_empty_vlr_record_ID, 0, 0, (add_empty_vlr_description ? FALSE : TRUE), add_empty_vlr_description);
            }

            // do we need an extra pass

            BOOL extra_pass = laswriteopener.is_piped();

            // we only really need an extra pass if the coordinates are altered or if points are filtered

            if (extra_pass)
            {
                if ((subsequence_start == 0) && (subsequence_stop == I64_MAX) && (clip_to_bounding_box == false) && (reproject_quantizer == 0) && (lasreadopener.get_filter() == 0) && ((lasreadopener.get_transform() == 0) || ((lasreadopener.get_transform()->transformed_fields & LASTRANSFORM_XYZ_COORDINATE) == 0)) && lasreadopener.get_filter() == 0)
                {
                    extra_pass = FALSE;
                }
            }

            // for piped output we need an extra pass

            if (extra_pass)
            {
                if (lasreadopener.is_piped())
                {
                    fprintf(stderr, "ERROR: input and output cannot both be piped\n");
                    usage(true);
                }

#ifdef _WIN32
                if (verbose) fprintf(stderr, "extra pass for piped output: reading %I64d points ...\n", lasreader->npoints);
#else
                if (verbose) fprintf(stderr, "extra pass for piped output: reading %lld points ...\n", lasreader->npoints);
#endif

                // maybe seek to start position

                if (subsequence_start) lasreader->seek(subsequence_start);

                while (lasreader->read_point())
                {
                    if (lasreader->p_count > subsequence_stop) break;

                    if (clip_to_bounding_box)
                    {
                        if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
                        {
                            continue;
                        }
                    }

                    if (reproject_quantizer)
                    {
                        lasreader->point.compute_coordinates();
                        geoprojectionconverter.to_target(lasreader->point.coordinates);
                        lasreader->point.compute_XYZ(reproject_quantizer);
                    }
                    lasinventory.add(&lasreader->point);
                }
                lasreader->close();

                if (reproject_quantizer) lasreader->header = *reproject_quantizer;

                lasinventory.update_header(&lasreader->header);

                if (verbose) { fprintf(stderr, "extra pass took %g sec.\n", taketime() - start_time); start_time = taketime(); }
#ifdef _WIN32
                if (verbose) fprintf(stderr, "piped output: reading %I64d and writing %I64d points ...\n", lasreader->npoints, lasinventory.extended_number_of_point_records);
#else
                if (verbose) fprintf(stderr, "piped output: reading %lld and writing %lld points ...\n", lasreader->npoints, lasinventory.extended_number_of_point_records);
#endif
            }
            else
            {
                if (reproject_quantizer)
                {
                    saved_quantizer = new LASquantizer();
                    *saved_quantizer = lasreader->header;
                    lasreader->header = *reproject_quantizer;
                }
#ifdef _WIN32
                if (verbose) fprintf(stderr, "reading %I64d and writing all surviving points ...\n", lasreader->npoints);
#else
                if (verbose) fprintf(stderr, "reading %lld and writing all surviving points ...\n", lasreader->npoints);
#endif
            }

            // check output

            if (!laswriteopener.active())
            {
                // create name from input name
                laswriteopener.make_file_name(lasreadopener.get_file_name());
            }
            else
            {
                // make sure we do not corrupt the input file

                if (lasreadopener.get_file_name() && laswriteopener.get_file_name() && (strcmp(lasreadopener.get_file_name(), laswriteopener.get_file_name()) == 0))
                {
                    fprintf(stderr, "ERROR: input and output file name are identical: '%s'\n", lasreadopener.get_file_name());
                    usage(true);
                }
            }

            // prepare the header for the surviving points
            memset(lasreader->header.system_identifier, 0, 31);
            strncpy(lasreader->header.system_identifier, "XD LASToLAS", 11);
            lasreader->header.system_identifier[31] = '\0';
            char temp[64];
#ifdef _WIN64
            sprintf(temp, "las2las64 (version %d)", LAS_TOOLS_VERSION);
#else // _WIN64
            sprintf(temp, "las2las (version %d)", LAS_TOOLS_VERSION);
#endif // _WIN64
            memset(lasreader->header.generating_software, 0, 32);
            strncpy(lasreader->header.generating_software, temp, 32);
            lasreader->header.generating_software[31] = '\0';

            // open laswriter

            LASwriter* laswriter = laswriteopener.open(&lasreader->header);

            if (laswriter == 0)
            {
                fprintf(stderr, "ERROR: could not open laswriter\n");
                byebye(true, argc == 1);
            }

            // for piped output we need to re-open the input file

            if (extra_pass)
            {
                if (!lasreadopener.reopen(lasreader))
                {
                    fprintf(stderr, "ERROR: could not re-open lasreader\n");
                    byebye(true);
                }
            }
            else
            {
                if (reproject_quantizer)
                {
                    lasreader->header = *saved_quantizer;
                    delete saved_quantizer;
                }
            }

            // maybe seek to start position

            if (subsequence_start) lasreader->seek(subsequence_start);

            // loop over points
            //if (nEPSG > 0) 
              //  toWGS84.Create("epsg:" + std::to_string(nEPSG), "epsg:4326");
            //else 
                toWGS84.Create("epsg:" + std::to_string(source), "epsg:4326");
            
            if (isDest) 
                toTarget.Create("epsg:4326", "epsg:" + std::to_string(target));
            //else if (nEPSG > 0)
              //  toTarget.Create("epsg:4326", "epsg:" + std::to_string(nEPSG));
            else
                toTarget.Create("epsg:4326", "epsg:" + std::to_string(source));
            
            Vec3d sourceVec,targetVec;
            
            if (point)
            {
                GeoidHeight geo;
                double lat,lon,hei;

                if (strcmp(geoId.c_str(), "") != 0) {

                    if (strcmp(geoId.c_str(), "N60") == 0)
                        geo.init("FIN2005N00.asc", TYPE_GEOID_N60);
                    else if (strcmp(geoId.c_str(), "N2000") == 0)
                        geo.init("FIN2000_block.asc", TYPE_GEOID_N2000);
                }

                while (lasreader->read_point())
                {
                    if (lasreader->p_count > subsequence_stop) break;

                    if (clip_to_bounding_box)
                    {
                        if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
                        {
                            continue;
                        }
                    }

                    if (isCrop)
                    {
                        if (!lasreader->point.inside_rectangle(box[0],box[1],box[2],box[3]))
                        {
                            continue;
                        }
                    }

                    else if (reproject_quantizer)
                    {
                        lasreader->point.compute_coordinates();
                        toWGS84.Convert(lasreader->point.coordinates[1], lasreader->point.coordinates[0], lat, lon);
                        //geoprojectionconverter.to_target(lasreader->point.coordinates);
                        if (strcmp(geoId.c_str(), "") != 0) {
                            geo.getHeightLinear(lat, lon, geoId, hei);
                        }
                        //lasreader->point.compute_XYZ(reproject_quantizer);
                        if (target != 4326) {
                            toTarget.Convert(lat, lon, lat, lon); //trans.Convert(y,x,lat,lon)
                            lasreader->point.set_X(reproject_quantizer->get_X(lat));
                            lasreader->point.set_Y(reproject_quantizer->get_Y(lon));
                        }
                        else {
                            lasreader->point.set_X(reproject_quantizer->get_X(lon));
                            lasreader->point.set_Y(reproject_quantizer->get_Y(lat));
                        }
                        if (strcmp(geoId.c_str(), "") != 0)  lasreader->point.set_Z(reproject_quantizer->get_Z(hei));;
                    }

                    *point = lasreader->point;

                    laswriter->write_point(point);
                    // without extra pass we need inventory of surviving points
                    if (!extra_pass) laswriter->update_inventory(point);
                }
                delete point;
                point = 0;
            }
            else
            {
                GeoidHeight geo;
                
                if (strcmp(geoId.c_str(), "") != 0) {

                    if (strcmp(geoId.c_str(), "N60") == 0)
                        geo.init("FIN2005N00.asc", TYPE_GEOID_N60);
                    else if (strcmp(geoId.c_str(), "N2000") == 0)
                        geo.init("FIN2000_block.asc", TYPE_GEOID_N2000); 
                }
                
                while (lasreader->read_point())
                {
                    if (lasreader->p_count > subsequence_stop) break;

                    if (clip_to_bounding_box)
                    {
                        if (!lasreader->point.inside_box(lasreader->header.min_x, lasreader->header.min_y, lasreader->header.min_z, lasreader->header.max_x, lasreader->header.max_y, lasreader->header.max_z))
                        {
                            continue;
                        }
                    }
                    if (isCrop)
                    {
                        lasreader->point.compute_coordinates();
                        if (!lasreader->point.inside_rectangle(box[0], box[1], box[2], box[3]))
                        {
                            continue;
                        }
                    }
                    else if (reproject_quantizer)
                    {
                            lasreader->point.compute_coordinates();

                            sourceVec.set(lasreader->point.coordinates[0], lasreader->point.coordinates[1], lasreader->point.coordinates[2]);
                            targetVec.set(0,0,0);
                            tranform(sourceVec,targetVec,toTarget,toWGS84,geoId,&m_GeoidPROJ);
                            

                            /*
                            toWGS84.Convert(lasreader->point.coordinates[1], lasreader->point.coordinates[0], lat, lon);
                           // geoprojectionconverter.to_target(lasreader->point.coordinates);
                            if (strcmp(geoId.c_str(), "") != 0) {
                                geo.getHeightLinear(lat, lon, geoId, hei);
                            }
                            hei += lasreader->point.coordinates[2];
                           // lasreader->point.compute_XYZ(reproject_quantizer);
                           */
                            if (target != 4326) {
                                lasreader->point.set_X(reproject_quantizer->get_X(targetVec[0]));
                                lasreader->point.set_Y(reproject_quantizer->get_Y(targetVec[1]));
                            }
                            else {
                                lasreader->point.set_X(reproject_quantizer->get_X(targetVec[1]));
                                lasreader->point.set_Y(reproject_quantizer->get_Y(targetVec[0]));
                            }

                            lasreader->point.set_Z(reproject_quantizer->get_Z(targetVec[2]));
                            /*
                            lasreader->point.set_X(reproject_quantizer->get_X(targetVec[0]));
                            lasreader->point.set_Y(reproject_quantizer->get_Y(targetVec[1]));
                            
                            if (strcmp(geoId.c_str(), "") != 0)  //lasreader->point.set_Z(reproject_quantizer->get_Z(hei));
                            else lasreader->point.set_Z(reproject_quantizer->get_Z(lasreader->point.coordinates[2]));
                            */
                    }

                    

                    laswriter->write_point(&lasreader->point);
                    // without extra pass we need inventory of surviving points
                    if (!extra_pass) laswriter->update_inventory(&lasreader->point);
                }
            }

            // without the extra pass we need to fix the header now

            if (!extra_pass)
            {
                if (reproject_quantizer) lasreader->header = *reproject_quantizer;
                laswriter->update_header(&lasreader->header, TRUE);
                if (verbose) { fprintf(stderr, "total time: %g sec. written %u surviving points to '%s'.\n", taketime() - start_time, (U32)laswriter->p_count, laswriteopener.get_file_name()); }
            }
            else
            {
                if (verbose) { fprintf(stderr, "main pass took %g sec.\n", taketime() - start_time); }
            }


            

            laswriter->close();
            // delete empty output files
            if (remove_empty_files && (laswriter->npoints == 0) && laswriteopener.get_file_name())
            {
                remove(laswriteopener.get_file_name());
                if (verbose) fprintf(stderr, "removing empty output file '%s'\n", laswriteopener.get_file_name());
            }
            delete laswriter;

            lasreader->close();
            delete lasreader;

            if (reproject_quantizer) delete reproject_quantizer;

            laswriteopener.set_file_name(0);

         
            //Save information of new transform file
            if (strcmp(pMetaDestFileName,"") != 0) {
                lasreader = lasreadopener.open(DestFile);

                if (lasreader == 0)
                {
                    fprintf(stderr, "ERROR: could not open lasreader\n");
                    usage(true, argc == 1);
                }
                getInfor(lasreader, &geoprojectionconverter, pMetaDestFileName, nEPSG, strCS);
                break;
            }
           
        }
        catch (...)
        {
            fprintf(stderr, "ERROR processing file '%s'. maybe file is corrupt?\n", lasreadopener.get_file_name());
            error = true;

            laswriteopener.set_file_name(0);
        }
    }

//    byebye(error, argc == 1);

    return 0;
}

int LasToLas::LasToTileset(const AppConfig m_AppConfig, const std::string& input,const std::string& output,const std::string& EPSG, const std::string& geo_id, const double cell_size, const double geometry_scale) const {
    GeoidHeight m_GeoidHeight;
    GeoidPROJ m_GeoidPROJ;

    if (!m_GeoidHeight.init(m_AppConfig.getPathN2000().c_str(), TYPE_GEOID_N2000))
        fprintf(stderr, "{\"Status\":\"OK\",\"Warning\": \"database caching FIN2005N00.asc not found: ", m_AppConfig.getPathN2000(), "\"}\n");

    if (!m_GeoidHeight.init(m_AppConfig.getPathN60().c_str(), TYPE_GEOID_N60))
        fprintf(stderr, "{\"Status\":\"OK\",\"Warning\": \"database caching FIN2000.asc not found", m_AppConfig.getPathN60(), "\"}\n");

    if (!m_GeoidPROJ.Create(&m_GeoidHeight, m_AppConfig.getPathConfig(), m_AppConfig.getCatalogPROJ()))
        fprintf(stderr, "{\"Status\":\"OK\",\"Warning\": \"geoid catalog file geoid.ini not found\"}\n");
    m_GeoidPROJ.setGeoid(geo_id);
    PointCloudModel pc("epsg:"+EPSG, geo_id); //Chua cong geoId thi de trong nua sau (epsg:3879,)
    BoundingBox boundingbox;
    pc.ReadPointCloud(input, boundingbox, &m_GeoidPROJ);
    QuadTreeObjectSizeBuilder quad(&m_GeoidPROJ, &pc, cell_size, geometry_scale);
    return quad.writeLandXmlObjectsToTileset(output + "/tileset.json", output);
}