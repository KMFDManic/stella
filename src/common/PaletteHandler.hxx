//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef PALETTE_HANDLER_HXX
#define PALETTE_HANDLER_HXX

#include "bspf.hxx"
#include "OSystem.hxx"

class PaletteHandler
{
  public:
    static constexpr const char* SETTING_STANDARD = "standard";
    static constexpr const char* SETTING_Z26 = "z26";
    static constexpr const char* SETTING_USER = "user";
    static constexpr const char* SETTING_CUSTOM = "custom";

    enum DisplayType {
      NTSC,
      PAL,
      SECAM,
      NumDisplayTypes
    };

    struct Adjustable {
      uInt32 hue, saturation, contrast, brightness, gamma;
    };

  public:
    PaletteHandler(OSystem& system);
    virtual ~PaletteHandler() = default;

    /**
      Switch between the available palettes.
    */
    void changePalette(bool increase = true);

    void selectAdjustable(bool next = true);
    void changeAdjustable(bool increase = true);


    void loadConfig(const Settings& settings);
    void saveConfig(Settings& settings) const;
    void setAdjustables(Adjustable& adjustable);
    void getAdjustables(Adjustable& adjustable) const;

    /**
      Change the "phase shift" variable.
      Note that there are two of these (NTSC and PAL).  The currently
      active mode will determine which one is used.

      @param increase increase if true, else decrease.
    */
    void changeColorPhaseShift(bool increase = true);

    /**
      Sets the palette according to the given palette name.

      @param palette  The palette to switch to.
    */
    void setPalette(const string& name);


    /**
      Sets the palette from current settings.
    */
    void setPalette();

    /**
      Generates a custom palette, based on user defined phase shifts.
    */
    void generateCustomPalette(ConsoleTiming timing);

  private:
    enum PaletteType {
      Standard,
      Z26,
      User,
      Custom,
      NumTypes,
      MinType = Standard,
      MaxType = Custom
    };

    float scaleFrom100(float x) const { return (x / 100.F) - 1.F; }
    uInt32 scaleTo100(float x) const { return uInt32(100 * (x + 1.F)); }

    PaletteType toPaletteType(const string& name) const;
    string toPaletteName(PaletteType type) const;

    PaletteArray adjustPalette(const PaletteArray& source);

    void changeSaturation(int& R, int& G, int& B, float change);

    /**
      Loads a user-defined palette file (from OSystem::paletteFile), filling the
      appropriate user-defined palette arrays.
    */
    void loadUserPalette();


  private:
    static const int NUM_ADJUSTABLES = 6;

    OSystem& myOSystem;

    uInt32 myCurrentAdjustable{0};
    struct AdjustableTag {
      const char* const type{nullptr};
      float* value{nullptr};
    };
    const std::array<AdjustableTag, NUM_ADJUSTABLES> myAdjustables =
    { {
      { "contrast", &myContrast },
      { "brightness", &myBrightness },
      { "hue", &myHue },
      { "saturation", &mySaturation },
      { "gamma", &myGamma },
      { "phase shift", nullptr },
    } };

    // range -1.0 to +1.0 (as in AtariNTSC)
    // Basic parameters
    float myContrast{0.0F};   // -1 = dark (0.5)       +1 = light (1.5)
    float myHue{0.0F};        // -1 = -180 degrees     +1 = +180 degrees
    float mySaturation{0.0F}; // -1 = grayscale (0.0)  +1 = oversaturated colors (2.0)
    float myBrightness{0.0F}; // -1 = dark (0.5)       +1 = light (1.5)
    // Advanced parameters
    float myGamma{0.0F};      // -1 = dark (1.5)       +1 = light (0.5)

    // Indicates whether an external palette was found and
    // successfully loaded
    bool myUserPaletteDefined{false};

    // Table of RGB values for NTSC, PAL and SECAM
    static PaletteArray ourNTSCPalette;
    static PaletteArray ourPALPalette;
    static PaletteArray ourSECAMPalette;

    // Table of RGB values for NTSC, PAL and SECAM - Z26 version
    static PaletteArray ourNTSCPaletteZ26;
    static PaletteArray ourPALPaletteZ26;
    static PaletteArray ourSECAMPaletteZ26;

    // Table of RGB values for NTSC, PAL and SECAM - user-defined
    static PaletteArray ourUserNTSCPalette;
    static PaletteArray ourUserPALPalette;
    static PaletteArray ourUserSECAMPalette;

    // Table of RGB values for NTSC, PAL - custom-defined
    static PaletteArray ourCustomNTSCPalette;
    static PaletteArray ourCustomPALPalette;

  private:
    PaletteHandler() = delete;
    PaletteHandler(const PaletteHandler&) = delete;
    PaletteHandler(PaletteHandler&&) = delete;
    PaletteHandler& operator=(const PaletteHandler&) = delete;
    PaletteHandler& operator=(const PaletteHandler&&) = delete;
};

#endif // PALETTE_HANDLER_HXX
