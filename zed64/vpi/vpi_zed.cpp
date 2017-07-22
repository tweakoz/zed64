///////////////////////////////////////////////////////////////////////////////
// Zed64
// Copyright (C) 2015-2017 - Michael T. Mayers (michael@tweakoz.com)
// Licensed under the GPL (v3)
//  see https://www.gnu.org/licenses/gpl-3.0.txt
///////////////////////////////////////////////////////////////////////////////

extern "C" { 
#include <vpi_user.h>
}
#include <string>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <OpenImageIO/imageio.h>
OIIO_NAMESPACE_USING

///////////////////////////////////////////////////////////////////////////////

static int vpi_zed_compiletf(char*user_data)
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct LINE
{
    std::vector<uint32_t> _pixels;
};

///////////////////////////////////////////////////////////////////////////////

struct WRITER
{
    WRITER()
    {
        printf( "WRITER\n");
    }
    ~WRITER()
    {
        if( _lines.size() )
        {
            int width = _lines[0]._pixels.size();
            int height = _lines.size();

            const char *filename = "z64test.png";
            const int xres = width, yres = height;
            const int channels = 4; // RGBA
            uint32_t pixels[xres*yres];
            int index = 0;
            for( int iy=0; iy<_lines.size(); iy++ )
            {   const auto& L = _lines[iy]._pixels;
                for( int ix=0; ix<L.size(); ix++ )
                {
                    pixels[index++] = L[ix];
                }
            }
            ImageOutput *out = ImageOutput::create (filename);
            if (! out)
            return;
            ImageSpec spec (xres, yres, channels, TypeDesc::UINT8);
            out->open (filename, spec);
            out->write_image (TypeDesc::UINT8, pixels);
            out->close ();
            //ImageOutput::destroy (out);
        }

        printf( "~WRITER\n");
    }
    std::vector<LINE> _lines;
};

///////////////////////////////////////////////////////////////////////////////

static int vpi_zed_calltf(char*user_data) {
    
    static WRITER wr;
    static LINE line;

    struct t_vpi_value argval;

    /////////////////////////////////////////

    auto systfref = vpi_handle(vpiSysTfCall, NULL);
    auto args_iter = vpi_iterate(vpiArgument, systfref);

    /////////////////////////////////////////

    auto getint = [&]()->int{
        auto argh = vpi_scan(args_iter);
        argval.format = vpiIntVal;
        vpi_get_value(argh, &argval);
        return (int) argval.value.integer;
    };

    /////////////////////////////////////////

    int chipaddr = getint();
    int chipdata = getint();
    int Hsize = getint();
    int is_blank = getint();
    int vgaH = getint();
    int vgaV = getint();
    int vgaR = getint();
    int vgaG = getint();
    int vgaB = getint();
    static int prv_blank = -1;

    /////////////////////////////////////////

    uint32_t pixel = (255<<24)
                   | (vgaB<<20)
                   | (vgaG<<12)
                   | (vgaR<<4);

    //printf( "is_blank<%d> pixel<%08x> chipaddr<%04x> chipdata<%04x>\n", is_blank, pixel, chipaddr, chipdata );

    if( is_blank )
    {
        int linecount = wr._lines.size();

        if( line._pixels.size() )
        {
            printf( "linecount<%d> npix<%d>\n", linecount, (int)line._pixels.size() );
            wr._lines.push_back(line);
        }
        line._pixels.clear();

        if(linecount==32)
            exit(0);
    }
    else
    {
        int X = line._pixels.size();

        if( X<Hsize )
        {
            line._pixels.push_back(pixel);
        }

    }

    prv_blank = is_blank;

    //vpi_printf("chip_addr<%04x> chip_data<%02x> vgaH<%d> vgaV<%d>\n", chipaddr, chipdata, vgaH, vgaV );

    vpi_free_object(args_iter);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void vpi_zed_register() {

      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$vpi_zed";
      tf_data.calltf    = vpi_zed_calltf;
      tf_data.compiletf = vpi_zed_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = 0;
      vpi_register_systf(&tf_data);
}

///////////////////////////////////////////////////////////////////////////////

extern "C" {

    void (*vlog_startup_routines[])() = {
        vpi_zed_register,
        0
    };

}