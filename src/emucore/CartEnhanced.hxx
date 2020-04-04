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

#ifndef CARTRIDGEENHANCED_HXX
#define CARTRIDGEENHANCED_HXX

class System;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  Enhanced cartridge base class used for multiple cart types.

  @author  Thomas Jentzsch
*/
class CartridgeEnhanced : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param md5       The md5sum of the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    CartridgeEnhanced(const ByteBuffer& image, size_t size, const string& md5,
                const Settings& settings);
    virtual ~CartridgeEnhanced() = default;

  public:
    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system) override;

    /**
      Reset device to its power-on state
    */
    void reset() override;


    /**
      Install pages for the specified bank in the system.

      @param bank The bank that should be installed in the system
    */
    bool bank(uInt16 bank, uInt16 segment);

    /**
      Install pages for the specified bank in the system.

      @param bank The bank that should be installed in the system
    */
    bool bank(uInt16 bank) override { return this->bank(bank, 0); }

    /**
      Get the current bank.

      @param address The address to use when querying the bank
    */
    uInt16 getBank(uInt16 address = 0) const override;

    /**
      Query the number of banks supported by the cartridge.
    */
    uInt16 bankCount() const override;

    /**
      Patch the cartridge ROM.

      @param address  The ROM address to patch
      @param value    The value to place into the address
      @return    Success or failure of the patch operation
    */
    bool patch(uInt16 address, uInt8 value) override;

    /**
      Access the internal ROM image for this cartridge.

      @param size  Set to the size of the internal ROM image data
      @return  A pointer to the internal ROM image data
    */
    const uInt8* getImage(size_t& size) const override;

    /**
      Save the current state of this cart to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const override;

    /**
      Load the current state of this cart from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in) override;

  public:
    /**
      Get the byte at the specified address.

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address) override;

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
      @return  True if the poke changed the device address space, else false
    */
    bool poke(uInt16 address, uInt8 value) override;

  protected:
    // The '2 ^ N = bank segment size' exponent
    uInt16 myBankShift{BANK_SHIFT};             // default 12 (-> one 4K segment)

    // The size of a bank's segment
    uInt16 myBankSize{0};

    // The mask for a bank segment
    uInt16 myBankMask{0};

    // The number of segments a bank is split into
    uInt16 myBankSegs{0};

    // The extra RAM size
    uInt16 myRamSize{RAM_SIZE};                 // default 0

    // The mask for the extra RAM
    uInt16 myRamMask{0};                        // RAM_SIZE - 1, but doesn't matter when RAM_SIZE is 0

    // Pointer to a dynamically allocated ROM image of the cartridge
    ByteBuffer myImage{nullptr};

    // Contains the offset into the ROM image for each of the bank segments
    DWordBuffer myCurrentSegOffset{nullptr};

    // Indicates whether to use direct ROM peeks or not
    bool myDirectPeek{true};

    // Pointer to a dynamically allocated RAM area of the cartridge
    ByteBuffer myRAM{nullptr};

    // The size of the ROM image
    size_t mySize{0};

  private:
    // Calculated as: log(ROM bank segment size) / log(2)
    static constexpr uInt16 BANK_SHIFT = 12;  // default = 4K

    // The size of extra RAM in ROM address space
    static constexpr uInt16 RAM_SIZE = 0;     // default = none

  protected:
    /**
      Check hotspots and switch bank if triggered.

      @param address  The address to check
      @param value    The optional value used to determine the bank switched to
      @return  True if a bank switch happened.
    */
    virtual bool checkSwitchBank(uInt16 address, uInt8 value = 0) = 0;

  private:
    /**
      Get the ROM's startup bank.

      @return  The bank the ROM will start in
    */
    virtual uInt16 getStartBank() const { return 0; }

    /**
      Get the hotspot in ROM address space.

      @return  The first hotspot address in ROM space or 0
    */
    virtual uInt16 romHotspot() const { return 0; }
    // TODO: handle cases where there the hotspots cover multiple pages

  private:
    // Following constructors and assignment operators not supported
    CartridgeEnhanced() = delete;
    CartridgeEnhanced(const CartridgeEnhanced&) = delete;
    CartridgeEnhanced(CartridgeEnhanced&&) = delete;
    CartridgeEnhanced& operator=(const CartridgeEnhanced&) = delete;
    CartridgeEnhanced& operator=(CartridgeEnhanced&&) = delete;
};

#endif