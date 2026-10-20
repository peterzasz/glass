#ifndef PLATEFILLER_H
#define PLATEFILLER_H

#include "data.h"
#include "cutter.h"

enum class FillMethod
{
    Fast,
    Rotation,
    PlacementLookaheadRotationEval,
    PlacementLookaheadFastEval,
};

class PlateFiller
{
    public:
        PlateFiller(FillMethod method, bool decr_area_order, const Bin& bin, Solution& s, Batch& batch) :    _method(method), 
                                                                                                        _decr_area_order(decr_area_order), 
                                                                                                        _bin(bin), 
                                                                                                        _s(s),
                                                                                                        _batch(batch) {}
    
        int fill()
        {
            switch(_method)
            {
                case FillMethod::Fast:
                    return cutter_fill(CutMethod::Fast);
                case FillMethod::Rotation:
                    return cutter_fill(CutMethod::Rotation);
                case FillMethod::PlacementLookaheadRotationEval:
                    return placement_lookahead_fill(FillMethod::Rotation);
                case FillMethod::PlacementLookaheadFastEval:
                    return placement_lookahead_fill(FillMethod::Fast);
            }

            return 0;
        }

    private:
        FillMethod _method;
        bool _decr_area_order;
        const Bin& _bin;
        Solution& _s;
        Batch& _batch;

        int cutter_fill(CutMethod cut_method);
        int placement_lookahead_fill(FillMethod evaluation_method);

        NextItem next_item();
        void initialize_stack_order();
        void order_stack();
};


#endif