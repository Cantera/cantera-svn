#ifndef CXX_IDEALGASMIX
#define CXX_IDEALGASMIX

#include <string>

#include "kernel/IdealGasPhase.h"
#include "kernel/GasKinetics.h"
#include "kernel/importKinetics.h"
#include "kernel/stringUtils.h"

namespace Cantera {

    class IdealGasMix : 
        public IdealGasPhase,
        public GasKinetics
    {
    public:

        IdealGasMix() : m_ok(false), m_r(0) {}

        IdealGasMix(std::string infile, std::string id="") : 
            m_ok(false), m_r(0) {
            
            m_r = get_XML_File(infile);
            m_id = id;
            if (id == "-") id = "";
            m_ok = buildSolutionFromXML(*m_r,
                m_id, "phase", this, this);
            if (!m_ok) throw CanteraError("IdealGasMix",
                "Cantera::buildSolutionFromXML returned false");
        }


        IdealGasMix(XML_Node& root,
            std::string id) : m_ok(false), m_r(&root), m_id(id) {
            m_ok = buildSolutionFromXML(root, id, "phase", this, this);
        }

        IdealGasMix(const IdealGasMix& other) : m_ok(false), 
                                                m_r(other.m_r),
                                                m_id(other.m_id) {
            m_ok = buildSolutionFromXML(*m_r, m_id, "phase", this, this);
        }

        virtual ~IdealGasMix() {}

        bool operator!() { return !m_ok;}
        bool ready() const { return m_ok; }
        friend std::ostream& operator<<(std::ostream& s, IdealGasMix& mix) {
            std::string r = mix.report(true);
            s << r;
            return s;
        }

            
    protected:
        bool m_ok;
        XML_Node* m_r;
        std::string m_id;

    private:
    };
}


#endif
