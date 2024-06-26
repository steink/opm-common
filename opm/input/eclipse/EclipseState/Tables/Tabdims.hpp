/*
  Copyright (C) 2015 Statoil ASA

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TABDIMS_HPP
#define TABDIMS_HPP

#include <cstddef>

/*
  The Tabdims class is a small utility class designed to hold on to
  the values from the TABDIMS keyword.
*/

namespace Opm {

    class Deck;
    class DeckKeyword;
    class DeckRecord;

    class Tabdims {
    public:

        /*
          The TABDIMS keyword has a total of 25 items; most of them
          are ECLIPSE300 only and quite exotic. Here we only
          internalize the most common items.
        */
        Tabdims();

        explicit Tabdims(const Deck& deck);

        static Tabdims serializationTestObject()
        {
            Tabdims result;
            result.m_ntsfun = 1;
            result.m_ntpvt = 2;
            result.m_nssfun = 3;
            result.m_nppvt = 4;
            result.m_ntfip = 5;
            result.m_nrpvt = 6;
            result.m_neosres = 7;
            result.m_neossur = 8;

            return result;
        }

        size_t getNumSatTables() const {
            return m_ntsfun;
        }

        size_t getNumPVTTables() const {
            return m_ntpvt;
        }

        size_t getNumSatNodes() const {
            return m_nssfun;
        }

        size_t getNumPressureNodes() const {
            return m_nppvt;
        }

        size_t getNumFIPRegions() const {
            return m_ntfip;
        }

        size_t getNumRSNodes() const {
            return m_nrpvt;
        }

        size_t getNumEosRes() const {
            return m_neosres;
        }

        size_t getNumEosSur() const {
            return m_neossur;
        }

        bool operator==(const Tabdims& data) const {
            return this->getNumSatTables() == data.getNumSatTables() &&
                   this->getNumPVTTables() == data.getNumPVTTables() &&
                   this->getNumSatNodes() == data.getNumSatNodes() &&
                   this->getNumPressureNodes() == data.getNumPressureNodes() &&
                   this->getNumFIPRegions() == data.getNumFIPRegions() &&
                   this->getNumRSNodes() == data.getNumRSNodes() &&
                   this->getNumEosRes() == data.getNumEosRes() &&
                   this->getNumEosSur() == data.getNumEosSur();
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_ntsfun);
            serializer(m_ntpvt);
            serializer(m_nssfun);
            serializer(m_nppvt);
            serializer(m_ntfip);
            serializer(m_nrpvt);
            serializer(m_neosres);
            serializer(m_neossur);
        }

    private:
        size_t m_ntsfun,m_ntpvt,m_nssfun,m_nppvt,m_ntfip,m_nrpvt, m_neosres, m_neossur;
    };
}


#endif
