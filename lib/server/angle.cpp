/*
 * angle.cpp
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 7/24/2015
 * 
 * Description:
 * Simple angle class
*/

Angle::Angle(double angle): _angle(angle) {
    if(_angle >= pi2()){
        _angle -= pi2();
    }
    else if(_angle < 0){
        _angle += pi2();
    }
}

Angle & Angle::operator+=(const Angle & other){
    _angle += other._angle;
    if(_angle >= pi2()){
        _angle -= pi2();
    }
    return this;
}

Angle & Angle::operator-=(const Angle & other){
    _angle -= other._angle;
    if(_angle < 0){
        _angle += pi2();
    }
    return this;
}

Angle Angle::operator+(const Angle & other) const{
    return Angle(*this) += other;
}

Angle Angle::operator-(const Angle & other) const{
    return Angle(*this) -= other;
}