#ifndef PLATEFILLER_H
#define PLATEFILLER_H

#include "data.h"

enum class FillMethod
{
    Fast,
    Rotation,
    PlacementLookahead
};

class Filler
{
    public:
        Filler(FillMethod method, bool decr_area_order, Bin& bin, Solution& s, Batch& batch) :  _method(method), 
                                                                                                _decr_area_order(decr_area_order), 
                                                                                                _bin(bin), 
                                                                                                _s(s),
                                                                                                _batch(batch) {}
    
        int fill()
        {
            switch(_method)
            {
                case FillMethod::Fast:
                    return fast_fill();
                case FillMethod::Rotation:
                    return rotation_fill();
                case FillMethod::PlacementLookahead:
                    return placement_lookahead_fill();
            }
        }

    private:
        FillMethod _method;
        bool _decr_area_order;
        Bin& _bin;
        Solution& _s;
        Batch& _batch;

        int fast_fill();
        int rotation_fill();
        int placement_lookahead_fill();

        NextItem next_item();
};


#endif