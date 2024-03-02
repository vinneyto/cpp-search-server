#pragma once

#include "ptrvector.h"
#include "scopedptr.h"

// Тут можно подключить scopedptr.h и ptrvector.h,
// если они вам понадобятся.

#include <new>  // Для исключения bad_alloc
#include <vector>

// Щупальце
class Tentacle {
   public:
    explicit Tentacle(int id) noexcept : id_(id) {}

    explicit Tentacle(const Tentacle& other) {
        id_ = other.id_;
        linked_tentacle_ = other.linked_tentacle_;
    }

    int GetId() const noexcept { return id_; }

    Tentacle* GetLinkedTentacle() const noexcept { return linked_tentacle_; }
    void LinkTo(Tentacle& tentacle) noexcept { linked_tentacle_ = &tentacle; }
    void Unlink() noexcept { linked_tentacle_ = nullptr; }

   private:
    int id_ = 0;
    Tentacle* linked_tentacle_ = nullptr;
};

// Осьминог
class Octopus {
   public:
    Octopus() : Octopus(8) {}

    explicit Octopus(const Octopus& other) : tentacles_(other.tentacles_) {}

    explicit Octopus(int num_tentacles) {
        for (int i = 1; i <= num_tentacles; ++i) {
            ScopedPtr t(new Tentacle(i));

            tentacles_.GetItems().push_back(t.GetRawPtr());

            t.Release();
        }
    }

    Octopus& operator=(const Octopus& rhs) {
        if (this != &rhs) {
            PtrVector<Tentacle> tantacles_copy(rhs.tentacles_);

            tentacles_ = tantacles_copy;
        }

        return *this;
    }

    // Добавляет новое щупальце с идентификатором,
    // равным (количество_щупалец + 1):
    // 1, 2, 3, ...
    // Возвращает ссылку на добавленное щупальце
    Tentacle& AddTentacle() {
        Tentacle* tentacle = new Tentacle(tentacles_.GetItems().size() + 1);
        tentacles_.GetItems().push_back(tentacle);

        return *tentacle;
    }

    int GetTentacleCount() const noexcept {
        return static_cast<int>(tentacles_.GetItems().size());
    }

    const Tentacle& GetTentacle(size_t index) const {
        return *tentacles_.GetItems().at(index);
    }

    Tentacle& GetTentacle(size_t index) {
        return *tentacles_.GetItems().at(index);
    }

   private:
    // Вектор хранит указатели на щупальца. Сами объекты щупалец находятся в
    // куче
    PtrVector<Tentacle> tentacles_;
};