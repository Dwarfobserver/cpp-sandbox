
#pragma once


namespace sc {

    // TODO Align data with F, custom allocator, look for inline data

    template <class> class movable_function;

    template <class Res, class...Args>
    class movable_function<Res(Args...)> {
        using fun_t =         Res  (*) (Args...);
        using move_ctor_f_t = void (*) (char* newF, char* oldF);
        using destroy_f_t =   void (*) (char* f);
        using invoke_f_t =    Res  (*) (char* f, Args&&...args);

        template <typename F>
        static Res invoke_f(F* fn, Args&&... args) {
            return (*fn)(std::forward<Args>(args)...);
        }

        template <typename F>
        static void destroy_f(F* f) {
            f->~F();
        }

    public:
        movable_function() noexcept :
                invoke_f_(nullptr),
                destroy_f_(nullptr),
                data_(nullptr)
        {}

        ~movable_function() noexcept {
            if (is_valid()) destroy_f_(data_.get());
        }

        template <class F, class = std::enable_if_t<
                !std::is_pointer_v<F> &&
                !std::is_lvalue_reference_v<F>
        >>
        movable_function(F&& f) :
                invoke_f_   (reinterpret_cast<invoke_f_t>(invoke_f<F>)),
                destroy_f_  (reinterpret_cast<destroy_f_t>(destroy_f<F>)),
                data_(new char[sizeof(F)])
        {
            new (reinterpret_cast<F*>(data_.get())) F(std::move(f));
        }

        movable_function(fun_t f) :
                invoke_f_   (reinterpret_cast<invoke_f_t>(invoke_f<fun_t>)),
                destroy_f_  (reinterpret_cast<destroy_f_t>(destroy_f<fun_t>)),
                data_(new char[sizeof(void*)])
        {
            *reinterpret_cast<fun_t*>(data_.get()) = f;
        }

        movable_function(movable_function&& f) noexcept = default;
        movable_function(movable_function const&) = delete;

        movable_function& operator=(movable_function&& f) noexcept {
            if (is_valid()) destroy_f_(data_.get());

            invoke_f_    = f.invoke_f_;
            destroy_f_   = f.destroy_f_;
            data_        = std::move(f.data_);
        }
        movable_function& operator=(movable_function const&) = delete;


        Res operator()(Args&&...args) {
            return invoke_f_(data_.get(), std::forward<Args>(args)...);
        }

        bool is_valid() const noexcept {
            return data_ != nullptr;
        }
        operator bool() const noexcept {
            return is_valid();
        }

        void swap(movable_function& f) noexcept {
            const auto invoke  = invoke_f_;
            const auto destroy = destroy_f_;
            auto data          = std::move(data_);

            invoke_f_  = f.invoke_f_;
            destroy_f_ = f.destroy_f_;
            data_      = std::move(f.data_);

            f.invoke_f_  = invoke;
            f.destroy_f_ = destroy;
            f.data_      = std::move(data);
        }
    private:
        invoke_f_t invoke_f_;
        destroy_f_t destroy_f_;
        std::unique_ptr<char[]> data_;
    };

    template <class Res, class...Args>
    void swap(movable_function<Res, Args...>& f1, movable_function<Res, Args...>& f2) {
        f1.swap(f2);
    };

}
