
#pragma once

#include <utility>
#include <string>

namespace gst_pipeline_helper
{
    class GError_wrapper
    {
    public:
        GError_wrapper() = default;

        GError_wrapper( const GError_wrapper& op2 ) = delete;
        GError_wrapper& operator=( const GError_wrapper& op2 ) = delete;

        GError_wrapper( GError_wrapper&& op2 ) noexcept : err_{ std::exchange( op2.err_, nullptr ) } {}
        GError_wrapper& operator=( GError_wrapper&& op2 ) noexcept { reset(); err_ = std::exchange( op2.err_, nullptr ); return *this; }

        ~GError_wrapper() {
            reset();
        }

        GError** reset_and_get() noexcept {
            reset();
            return &err_;
        }
        void    reset() noexcept
        {
            if( err_ ) {
                g_error_free( err_ );
                err_ = nullptr;
            }
        }

        constexpr GError* get() const noexcept { return err_; }

        constexpr bool    is_error() const noexcept { return err_ != nullptr; }
        constexpr explicit operator bool() const noexcept { return is_error(); }

        std::string     to_string() const noexcept {
            if( !is_error() ) {
                return "No error";
            }
            return std::string( "Error: " ) + err_->message;
        }
    private:
        GError* err_ = nullptr;
    };
}