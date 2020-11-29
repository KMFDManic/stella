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

/*
 Formats (all optional):
   4,             ; score digits
   0,             ; trailing zeroes
   B,             ; score format (BCD, HEX)


   0,             ; invert score order

   B,             ; variation format (BCD, HEX)
   0,             ; zero-based variation
   "",            ; special label (5 chars)
   B,             ; special format (BCD, HEX)
   0,             ; zero-based special
 Addresses (in hex):
   n-times xx,    ; score info, high to low
   xx,            ; variation address (if more than 1 variation)
   xx             ; special address (if defined)

 TODO:
 - variation bits (Centipede)
 - score swaps (Asteroids)
 - special: one optional and named value extra per game (round, level...)
*/

#include <cmath>

#include "OSystem.hxx"
#include "PropsSet.hxx"
#include "System.hxx"
#include "Cart.hxx"
#include "Console.hxx"
#include "Launcher.hxx"
#include "Base.hxx"

#include "HighScoresManager.hxx"

using namespace BSPF;
using namespace std;
using namespace HSM;
using Common::Base;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HighScoresManager::HighScoresManager(OSystem& osystem)
  : myOSystem(osystem)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int16 HighScoresManager::peek(uInt16 addr) const
{
  if (myOSystem.hasConsole())
  {
    if(addr < 0x100u || myOSystem.console().cartridge().internalRamSize() == 0)
      return myOSystem.console().system().peek(addr);
    else
      return myOSystem.console().cartridge().internalRamGetValue(addr);
  }
  return NO_VALUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const json HighScoresManager::properties(const Properties& props) const
{
  const string& property = props.get(PropType::Cart_Highscore);

  if(property.empty())
    return json::array();

  return json::parse(property);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
json HighScoresManager::properties(json& jprops) const
{
  Properties props;

  if(myOSystem.hasConsole())
  {
    props = myOSystem.console().properties();
  }
  else
  {
    const string& md5 = myOSystem.launcher().selectedRomMD5();
    myOSystem.propSet().getMD5(md5, props);
  }

  return jprops = properties(props);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::enabled() const
{
  json hsProp;

  return properties(hsProp).contains(SCORE_ADDRESSES);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::numVariations(const json& jprops) const
{
  return min(getPropInt(jprops, VARIATIONS_COUNT, DEFAULT_VARIATION), MAX_VARIATIONS);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::get(const Properties& props, uInt32& numVariationsR,
                            ScoresInfo& info) const
{
  json jprops = properties(props);

  numVariationsR = numVariations(jprops);

  //info.armRAM = armRAM(jprops);
  info.numDigits = numDigits(jprops);
  info.trailingZeroes = trailingZeroes(jprops);
  info.scoreBCD = scoreBCD(jprops);
  info.scoreInvert = scoreInvert(jprops);
  info.varsBCD = varBCD(jprops);
  info.varsZeroBased = varZeroBased(jprops);
  info.special = specialLabel(jprops);
  info.specialBCD = specialBCD(jprops);
  info.specialZeroBased = specialZeroBased(jprops);
  info.notes = notes(jprops);

  info.varsAddr = varAddress(jprops);
  info.specialAddr = specialAddress(jprops);

  info.scoreAddr = getPropScoreAddr(jprops);

  //if(jprops.contains(SCORE_ADDRESSES))
  //{
  //  const json addrProps = jprops.at(SCORE_ADDRESSES);

  //  if(!addrProps.empty() && addrProps.is_array())
  //  {
  //    int a = 0;

  //    for(const json& addresses : addrProps)
  //    {
  //      const string address = addresses.get<string>();

  //      if(address.empty())
  //        info.scoreAddr[a++] = DEFAULT_ADDRESS;
  //      else
  //        info.scoreAddr[a++] = fromHexStr(address);
  //    }

  //  }
  //}

  return enabled();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void HighScoresManager::set(Properties& props, uInt32 numVariations,
                            const ScoresInfo& info) const
{
  json jprops = json::object();

  // handle variations
  jprops[VARIATIONS_COUNT] = min(numVariations, MAX_VARIATIONS);
  if(numVariations != DEFAULT_VARIATION)
    jprops[VARIATIONS_ADDRESS] = "0x" + Base::toString(info.varsAddr, Base::Fmt::_16);
  if(info.varsBCD != DEFAULT_VARS_BCD)
    jprops[VARIATIONS_BCD] = info.varsBCD;
  if(info.varsZeroBased != DEFAULT_VARS_ZERO_BASED)
    jprops[VARIATIONS_ZERO_BASED] = info.varsZeroBased;

  // handle score
  if(info.numDigits != DEFAULT_DIGITS)
    jprops[SCORE_DIGITS] = info.numDigits;
  if(info.trailingZeroes != DEFAULT_TRAILING)
    jprops[SCORE_TRAILING_ZEROES] = info.trailingZeroes;
  if(info.scoreBCD != DEFAULT_SCORE_BCD)
    jprops[SCORE_BCD] = info.scoreBCD;
  if(info.scoreInvert != DEFAULT_SCORE_REVERSED)
    jprops[SCORE_INVERTED] = info.scoreInvert;

  uInt32 addrBytes = numAddrBytes(info.numDigits, info.trailingZeroes);
  json addresses = json::array();

  for(uInt32 a = 0; a < addrBytes; ++a)
    addresses.push_back("0x" + Base::toString(info.scoreAddr[a], Base::Fmt::_16));
  jprops[SCORE_ADDRESSES] = addresses;

  // handle special
  if(!info.special.empty())
    jprops[SPECIAL_LABEL] = info.special;
  if(!info.special.empty())
    jprops[SPECIAL_ADDRESS] = "0x" + Base::toString(info.specialAddr, Base::Fmt::_16);
  if(info.specialBCD != DEFAULT_SPECIAL_BCD)
    jprops[SPECIAL_BCD] = info.specialBCD;
  if(info.specialZeroBased != DEFAULT_SPECIAL_ZERO_BASED)
    jprops[SPECIAL_ZERO_BASED] = info.specialZeroBased;

  // handle notes
  if(!info.notes.empty())
    jprops[NOTES] = info.notes;

  //if(info.armRAM != DEFAULT_ARM_RAM)
  //  jprops[""] = info.armRAM;

  props.set(PropType::Cart_Highscore, jprops.dump());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::numDigits(const json& jprops) const
{
  return min(getPropInt(jprops, SCORE_DIGITS, DEFAULT_DIGITS), MAX_SCORE_DIGITS);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::trailingZeroes(const json& jprops) const
{
  return min(getPropInt(jprops, SCORE_TRAILING_ZEROES, DEFAULT_TRAILING), MAX_TRAILING);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::scoreBCD(const json& jprops) const
{
  return getPropBool(jprops, SCORE_BCD, DEFAULT_SCORE_BCD);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::scoreInvert(const json& jprops) const
{
  return getPropBool(jprops, SCORE_INVERTED, DEFAULT_SCORE_REVERSED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::varBCD(const json& jprops) const
{
  return getPropBool(jprops, VARIATIONS_BCD, DEFAULT_VARS_BCD);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::varZeroBased(const json& jprops) const
{
  return getPropBool(jprops, VARIATIONS_ZERO_BASED, DEFAULT_VARS_ZERO_BASED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::specialLabel(const json& jprops) const
{
  return getPropStr(jprops, SPECIAL_LABEL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::specialBCD(const json& jprops) const
{
  return getPropBool(jprops, SPECIAL_BCD, DEFAULT_SPECIAL_BCD);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::specialZeroBased(const json& jprops) const
{
  return getPropBool(jprops, SPECIAL_ZERO_BASED, DEFAULT_SPECIAL_ZERO_BASED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::notes(const json& jprops) const
{
  return getPropStr(jprops, NOTES);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*bool HighScoresManager::armRAM(const json& jprops) const
{
  //string armRAM = getPropIdx(jprops, PropType::Cart_Formats, IDX_ARM_RAM);

  //return armRAM.empty() ? DEFAULT_ARM_RAM : armRAM != "0";
  return false;
}*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 HighScoresManager::varAddress(const json& jprops) const
{
  return getPropAddr(jprops, VARIATIONS_ADDRESS, DEFAULT_ADDRESS);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 HighScoresManager::specialAddress(const json& jprops) const
{
  return getPropAddr(jprops, SPECIAL_ADDRESS, DEFAULT_ADDRESS);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::numAddrBytes(Int32 digits, Int32 trailing) const
{
  return (digits - trailing + 1) / 2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::numAddrBytes(const json& jprops) const
{
  return numAddrBytes(numDigits(jprops), trailingZeroes(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::numVariations() const
{
  json jprops;
  uInt16 vars = numVariations(properties(jprops));

  return vars;;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::specialLabel() const
{
  json jprops;

  return specialLabel(properties(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::variation(uInt16 addr, bool varBCD, bool zeroBased,
                                   uInt32 numVariations) const
{
  if (!myOSystem.hasConsole())
    return DEFAULT_VARIATION;

  Int32 var = peek(addr);

  return convert(var, numVariations, varBCD, zeroBased);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::variation() const
{
  json jprops;
  uInt16 addr = varAddress(properties(jprops));

  if(addr == DEFAULT_ADDRESS) {
    if(numVariations() == 1)
      return DEFAULT_VARIATION;
    else
      return NO_VALUE;
  }

  return variation(addr, varBCD(jprops), varZeroBased(jprops), numVariations(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::score(uInt32 numAddrBytes, uInt32 trailingZeroes,
                               bool isBCD, const ScoreAddresses& scoreAddr) const
{
  if (!myOSystem.hasConsole())
    return NO_VALUE;

  Int32 totalScore = 0;

  for (uInt32 b = 0; b < numAddrBytes; ++b)
  {
    uInt16 addr = scoreAddr[b];
    Int32 score;

    totalScore *= isBCD ? 100 : 256;
    score = peek(addr);
    if (isBCD)
    {
      score = fromBCD(score);
      // verify if score is legit
      if (score == NO_VALUE)
        return NO_VALUE;
    }
    totalScore += score;
  }

  if (totalScore != NO_VALUE)
    for (uInt32 i = 0; i < trailingZeroes; ++i)
      totalScore *= 10;

  return totalScore;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::score() const
{
  json jprops;
  uInt32 numBytes = numAddrBytes(properties(jprops));
  const ScoreAddresses scoreAddr = getPropScoreAddr(jprops);

  if(uInt32(scoreAddr.size()) < numBytes)
    return NO_VALUE;
  return score(numBytes, trailingZeroes(jprops), scoreBCD(jprops), scoreAddr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::formattedScore(Int32 score, Int32 width) const
{
  if(score <= 0)
    return "";

  ostringstream buf;
  json jprops;
  Int32 digits = numDigits(properties(jprops));

  if(scoreBCD(jprops))
  {
    if(width > digits)
      digits = width;
    buf << std::setw(digits) << std::setfill(' ') << score;
  }
  else {
    if(width > digits)
      buf << std::setw(width - digits) << std::setfill(' ') << "";
    buf << std::hex << std::setw(digits) << std::setfill('0') << score;
  }

  return buf.str();
}

bool HighScoresManager::scoreInvert() const
{
  json jprops;
  return scoreInvert(properties(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::special() const
{
  json jprops;
  uInt16 addr = specialAddress(properties(jprops));

  if (addr == DEFAULT_ADDRESS)
    return NO_VALUE;

  return special(addr, specialBCD(jprops), specialZeroBased(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::special(uInt16 addr, bool varBCD, bool zeroBased) const
{
  if (!myOSystem.hasConsole())
    return NO_VALUE;

  Int32 var = peek(addr);

  if (varBCD)
    var = fromBCD(var);

  var += zeroBased ? 1 : 0;

  return var;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::notes() const
{
  json jprops;

  return notes(properties(jprops));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::convert(Int32 val, uInt32 maxVal, bool isBCD, bool zeroBased) const
{
  maxVal += zeroBased ? 0 : 1;
  Int32 bits = isBCD
    ? ceil(log(maxVal) / log(10) * 4)
    : ceil(log(maxVal) / log(2));

  // limit to maxVal's bits
  val %= 1 << bits;

  if (isBCD)
    val = fromBCD(val);

  if(val == NO_VALUE)
    return 0;

  val += zeroBased ? 1 : 0;

  return val;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool HighScoresManager::getPropBool(const json& jprops, const string& key,
                                    bool defVal) const
{
  return jprops.contains(key) ? jprops.at(key).get<bool>() : defVal;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 HighScoresManager::getPropInt(const json& jprops, const string& key,
                                     uInt32 defVal) const
{
  return jprops.contains(key) ? jprops.at(key).get<uInt32>() : defVal;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string HighScoresManager::getPropStr(const json& jprops, const string& key,
                                           const string& defVal) const
{
  return jprops.contains(key) ? jprops.at(key).get<string>() : defVal;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 HighScoresManager::getPropAddr(const json& jprops, const string& key,
                                      uInt16 defVal) const
{
  const string str = getPropStr(jprops, key);

  return str.empty() ? defVal : fromHexStr(str);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const HSM::ScoreAddresses HighScoresManager::getPropScoreAddr(const json& jprops) const
{
  ScoreAddresses scoreAddr{};

  if(jprops.contains(SCORE_ADDRESSES))
  {
    const json addrProps = jprops.at(SCORE_ADDRESSES);

    if(!addrProps.empty() && addrProps.is_array())
    {
      int a = 0;

      for(const json& addresses : addrProps)
      {
        const string address = addresses.get<string>();

        if(address.empty())
          scoreAddr[a++] = DEFAULT_ADDRESS;
        else
          scoreAddr[a++] = fromHexStr(address);
      }
    }
  }
  return scoreAddr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 HighScoresManager::fromHexStr(const string& addr) const
{
  string naked = addr;

  if(int pos = naked.find("0x") != std::string::npos)
    naked = naked.substr(pos + 1);

  return stringToIntBase16(naked);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 HighScoresManager::fromBCD(uInt8 bcd) const
{
  // verify if score is legit
  if ((bcd & 0xF0) >= 0xA0 || (bcd & 0xF) >= 0xA)
    return NO_VALUE;

  return (bcd >> 4) * 10 + bcd % 16;
}

const string HighScoresManager::VARIATIONS_COUNT = "variations_number";
const string HighScoresManager::VARIATIONS_ADDRESS = "variations_address";
const string HighScoresManager::VARIATIONS_BCD = "variations_bcd";
const string HighScoresManager::VARIATIONS_ZERO_BASED = "variations_zero_based";
const string HighScoresManager::SCORE_DIGITS = "score_digits";
const string HighScoresManager::SCORE_TRAILING_ZEROES = "score_trailing_zeroes";
const string HighScoresManager::SCORE_BCD = "score_bcd";
const string HighScoresManager::SCORE_INVERTED = "score_inverted";
const string HighScoresManager::SCORE_ADDRESSES = "score_addresses";
const string HighScoresManager::SPECIAL_LABEL = "special_label";
const string HighScoresManager::SPECIAL_ADDRESS = "special_address";
const string HighScoresManager::SPECIAL_BCD = "special_bcd";
const string HighScoresManager::SPECIAL_ZERO_BASED = "special_zero_based";
const string HighScoresManager::NOTES = "notes";