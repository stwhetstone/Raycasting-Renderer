#include <cmath>
#include <algorithm>

#ifndef VEC2F_H
#define VEC2F_H

template <typename T>
class Vec2 {
    public:
        float length;

        Vec2() {
            this->_x = 0;
            this->_y = 0;
            
            this->length = 0;
        }
        
        Vec2(T x, T y) {
            this->_x = x;
            this->_y = y;
            
            this->length = std::sqrt(this->_x * this->_x + this->_y * this->_y);
        }

        static void rotate(Vec2& v, Vec2& center, T angle) {
            T tmpx = v.x(), tmpy = v.y(); 
            v.x(center.x() + ((tmpx - center.x()) * cos(angle) - (tmpy - center.y()) * sin(angle)));
			v.y(center.y() + ((tmpx - center.x()) * sin(angle) + (tmpy - center.y()) * cos(angle)));
        }

        T x() {
            return this->_x;
        }
        
        void x(T x) {
            this->_x = x;
            this->length = std::sqrt(this->_x * this->_x + this->_y * this->_y);
        }

        T y() {
            return this->_y;
        }
        
        void y(T y) {
            this->_y = y;
            this->updateLength();
        }

        void limit(T max) {
            if(this->_x >= 0) {
                this->_x = std::min(this->_x, max);
            } else if(this->_x < 0) {
                this->_x = std::max(this->_x, -1.0f * max);
            }
        
            if(this->_y >= 0) {
                this->_y = std::min(this->_y, max);
            } else if(this->_y < 0) {
                this->_y = std::max(this->_y, -1.0f * max);
            }
        }

        void normalize() {
            this->_x /= this->length;
            this->_y /= this->length;
        }
        
        Vec2 operator + (Vec2 other) {
            return Vec2(this->_x + other.x(), this->_y + other.y());
        }

        Vec2 operator - (Vec2 other) {
            return Vec2(this->_x - other.x(), this->_y - other.y());
        }

        Vec2 operator * (T a) {
            return Vec2(this->_x * a, this->_y * a );
        }

        Vec2 operator / (T a) {
            return Vec2(this->_x / a, this->_y / a );
        }
        
        void operator += (Vec2 other) {
            this->_x += other.x();
            this->_y += other.y();
            
            this->updateLength();
        }

        void operator -= (Vec2 other) {
            this->_x -= other.x();
            this->_y -= other.y();
            
            this->updateLength();
        }

        void operator *= (T a) {
            this->_x *= a;
            this->_y *= a;
            
            this->updateLength();
        }

        void operator /= (T a) {
            this->_x /= a;
            this->_y /= a;
            
            this->updateLength();
        }

        void operator = (Vec2 a) {
            this->_x = a.x();
            this->_y = a.y();
            
            this->updateLength();
        }
        
    private:
        T _x, _y;
        
        void updateLength() {
            this->length = std::sqrt(this->_x * this->_x + _y * this->_y);

        }
    
        
        
};
#endif
