#ifndef PLATEFILLER_H
#define PLATEFILLER_H

#include "data.h"
#include "cutter.h"
#include "fillertypes.h"

class PlateFiller
{
    public:
        PlateFiller(FillerType filler_type, const Bin& bin, Solution& s, Batch& batch, int& items_placed) : _filler_type(filler_type), 
                                                                                                            _bin(bin), 
                                                                                                            _s(s),
                                                                                                            _batch(batch),
                                                                                                            _items_placed(items_placed) {}
    
        int fill()
        {
            switch(_filler_type.method)
            {
                case FillMethod::NoRotation:
                    return cutter_fill(CutMethod::NoRotation);
                case FillMethod::Rotation:
                    return cutter_fill(CutMethod::Rotation);
                case FillMethod::PlacementLookahead:
                    return placement_lookahead_fill({Ordering::AreaOrder,FillMethod::Rotation});
                case FillMethod::PlacementLookahead2:
                    return placement_lookahead_fill({Ordering::AreaOrder,FillMethod::PlacementLookahead});
                case FillMethod::OrderLookahead:
                    return order_lookahead_fill(1,{Ordering::AreaOrder,FillMethod::PlacementLookahead});
            }

            return 0;
        }

    private:
        FillerType _filler_type;
        const Bin& _bin;
        Solution& _s;
        Batch& _batch;
        int& _items_placed;

        int cutter_fill(CutMethod cut_method);
        int placement_lookahead_fill(FillerType evaluation_method);

        int order_lookahead_fill(int block_size, FillerType eval_method);
        void cut_out(int block_size, FillerType eval_method, Solution& _best_sol, Batch& _best_batch, int& best_val, int& items_val, int val);


        NextItem next_item();
        void initialize_stack_order();

        void order_stack_by_area();
        void order_stack_by_height_width();
        void order_stack_by_width_height();
};


#endif