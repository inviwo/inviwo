#include <ticpp/ticpprc.h>

#include <ticpp/ticpp.h>

TiCppRC::TiCppRC() : privRC{this}, m_tiRC{&privRC} {
    // Spawn reference counter for this object
    // m_tiRC = new TiCppRCImp(this);
}

void TiCppRC::DeleteSpawnedWrappers() {
    std::vector<ticpp::Base*>::reverse_iterator wrapper;
    for (wrapper = m_spawnedWrappers.rbegin(); wrapper != m_spawnedWrappers.rend(); ++wrapper) {
        delete *wrapper;
    }
    m_spawnedWrappers.clear();
}

TiCppRC::~TiCppRC() {
    DeleteSpawnedWrappers();

    // Set pointer held by reference counter to NULL
    this->m_tiRC->Nullify();

    // Decrement reference - so reference counter will delete itself if necessary
    this->m_tiRC->DecRef();
}

//*****************************************************************************

TiCppRCImp::TiCppRCImp(TiCppRC* tiCppRC) : m_count(1), m_tiCppRC(tiCppRC) {}

void TiCppRCImp::IncRef() { m_count++; }

void TiCppRCImp::DecRef() {
    m_count--;
    if (0 == m_count) {
        delete m_tiCppRC;
        //delete this;
    }
}

void TiCppRCImp::InitRef() { m_count = 1; }

void TiCppRCImp::Nullify() { m_tiCppRC = nullptr; }

TiCppRC* TiCppRCImp::Get() { return m_tiCppRC; }

bool TiCppRCImp::IsNull() { return nullptr == m_tiCppRC; }
