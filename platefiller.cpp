#include "cutter.h"
#include "data.h"
#include "platefiller.h"

NextItem Filler::next_item()
{
    for( int i = 0; i < _batch.stacks.size(); ++i )
    {
        if( _batch.stacks[i].can_cut && _batch.stacks[i].at < _batch.stacks[i].item_ids.size() )
        {
            _batch.stacks[i].at++;
            return {i, _batch.stacks[i].at-1};
        }
    }

    return {-1,-1};
}