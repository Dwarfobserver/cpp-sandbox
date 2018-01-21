
#pragma once


namespace sc {

    // TODO Align data with F, custom allocator, look for inline data

    template <class> class movable_function;

    template <class Res, class...Args>
    class movable_function<Res(Args...)> {
        using fun_t =         Res  (*) (Args...);
        using destroy_f_t =   void (*) (char* f);
        using invoke_f_t =    Res  (*) (char* f, Args&&...args);

        template <typename F>
        static Res invoke_f(F* f, Args&&... args) {
            return (*f)(std::forward<Args>(args)...);
        }
        template <typename F>
        static void destroy_f(F* f) {
            f->~F();
            std::allocator<F>().deallocate(f, 1);
        }

        static Res invoke_fun(char* f, Args&&...args) {
            return reinterpret_cast<fun_t>(f)(std::forward<Args>(args)...);
        }
        static void destroy_fun(char* f) {}

    public:
        movable_function() noexcept :
                invoke_f_(nullptr),
                destroy_f_(nullptr),
                data_(nullptr)
        {}

        ~movable_function() noexcept {
            destroy();
        }

        template <class F, class = std::enable_if_t<
                !std::is_pointer_v<F> &&
                !std::is_lvalue_reference_v<F>
        >>
        movable_function(F&& f) :
                invoke_f_ (reinterpret_cast<invoke_f_t>(invoke_f<F>)),
                destroy_f_(reinterpret_cast<destroy_f_t>(destroy_f<F>)),
                data_(reinterpret_cast<char*>(std::allocator<F>().allocate(1)))
        {
            new (reinterpret_cast<F*>(data_)) F(std::move(f));
        }
        movable_function(fun_t f) :
                invoke_f_ (invoke_fun),
                destroy_f_(destroy_fun),
                data_(reinterpret_cast<char*>(f))
        {
            *reinterpret_cast<fun_t*>(&data_) = f;
        }

        movable_function(movable_function const&) = delete;
        movable_function(movable_function&& f) noexcept :
                invoke_f_(f.invoke_f_),
                destroy_f_(f.destroy_f_),
                data_(f.data_)
        {
            f.data_ = nullptr;
        }

        movable_function& operator=(movable_function const&) = delete;
        movable_function& operator=(movable_function&& f) noexcept {
            destroy();
            invoke_f_  = f.invoke_f_;
            destroy_f_ = f.destroy_f_;
            data_      = f.data_;
            f.data_ = nullptr;
        }

        Res operator()(Args&&...args) {
            return invoke_f_(data_, std::forward<Args>(args)...);
        }

        bool is_valid() const noexcept {
            return data_ != nullptr;
        }
        operator bool() const noexcept {
            return is_valid();
        }

        void reset() noexcept {
            destroy();
        }

        void swap(movable_function& f) noexcept {
            const auto invoke  = invoke_f_;
            const auto destroy = destroy_f_;
            const auto data    = data_;

            invoke_f_  = f.invoke_f_;
            destroy_f_ = f.destroy_f_;
            data_      = f.data_;

            f.invoke_f_  = invoke;
            f.destroy_f_ = destroy;
            f.data_      = data;
        }
    private:
        invoke_f_t invoke_f_;
        destroy_f_t destroy_f_;
        char* data_;

        void destroy() noexcept {
            if (data_) {
                destroy_f_(data_);
                data_ = nullptr;
            }
        }
    };

    template <class Res, class...Args>
    void swap(movable_function<Res, Args...>& f1, movable_function<Res, Args...>& f2) {
        f1.swap(f2);
    };

}
