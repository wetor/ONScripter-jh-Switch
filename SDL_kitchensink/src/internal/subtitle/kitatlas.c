#include <assert.h>

#include "kitchensink/internal/subtitle/kitatlas.h"
#include "kitchensink/internal/utils/kitlog.h"

static int min(int a, int b) {
    if(a < b)
        return a;
    return b;
}


Kit_TextureAtlas* Kit_CreateAtlas() {
    Kit_TextureAtlas *atlas = calloc(1, sizeof(Kit_TextureAtlas));
    if(atlas == NULL) {
        goto exit_0;
    }
    atlas->cur_items = 0;
    atlas->max_items = 1024;
    atlas->max_shelves = 256;
    atlas->w = 0;
    atlas->h = 0;

    // Allocate items. These hold the surfaces that should be in atlas
    atlas->items = calloc(atlas->max_items, sizeof(Kit_TextureAtlasItem));
    if(atlas->items == NULL) {
        goto exit_1;
    }

    // Allocate shelves. These describe the used space of the atlas
    atlas->shelves = calloc(atlas->max_shelves, sizeof(Kit_Shelf));
    if(atlas->shelves == NULL) {
        goto exit_2;
    }

    return atlas;

exit_2:
    free(atlas->items);
exit_1:
    free(atlas);
exit_0:
    return NULL;
}

void Kit_ClearAtlasContent(Kit_TextureAtlas *atlas) {
    atlas->cur_items = 0;
    memset(atlas->items, 0, atlas->max_items * sizeof(Kit_TextureAtlasItem));
    memset(atlas->shelves, 0, atlas->max_shelves * sizeof(Kit_Shelf));
}

void Kit_FreeAtlas(Kit_TextureAtlas *atlas) {
    assert(atlas != NULL);
    free(atlas->items);
    free(atlas->shelves);
    free(atlas);
}

void Kit_SetItemAllocation(Kit_TextureAtlasItem *item, SDL_Surface *surface, int shelf, int slot, int x, int y) {
    assert(item != NULL);

    item->cur_shelf = shelf;
    item->cur_slot = slot;
    item->source.x = x;
    item->source.y = y;
    item->source.w = surface->w;
    item->source.h = surface->h;
}

int Kit_FindFreeAtlasSlot(Kit_TextureAtlas *atlas, SDL_Surface *surface, Kit_TextureAtlasItem *item) {
    assert(atlas != NULL);
    assert(item != NULL);

    int shelf_w;
    int shelf_h;
    int total_remaining_h = atlas->h;
    int total_reserved_h = 0;

    // First, try to look for a good, existing shelf
    int best_shelf_idx = -1;
    int best_shelf_h = atlas->h;
    int best_shelf_y = 0;

    // Try to find a good shelf to put this item in
    int shelf_idx;
    for(shelf_idx = 0; shelf_idx < atlas->max_shelves; shelf_idx++) {
        shelf_w = atlas->shelves[shelf_idx].width;
        shelf_h = atlas->shelves[shelf_idx].height;
        if(shelf_h == 0) {
            break;
        }
        total_remaining_h -= shelf_h;
        total_reserved_h += shelf_h;

        // If the item fits, check if the space is better than previous one
        if(surface->w <= (atlas->w - shelf_w) && surface->h <= shelf_h && shelf_h < best_shelf_h) {
            best_shelf_h = shelf_h;
            best_shelf_idx = shelf_idx;
            best_shelf_y = total_reserved_h - shelf_h;
        }
    }

    // If existing shelf found, put the item there. Otherwise create a new shelf.
    if(best_shelf_idx != -1) {
        Kit_SetItemAllocation(
            item,
            surface,
            best_shelf_idx,
            atlas->shelves[best_shelf_idx].count,
            atlas->shelves[best_shelf_idx].width,
            best_shelf_y);
        atlas->shelves[best_shelf_idx].width += surface->w;
        atlas->shelves[best_shelf_idx].count += 1;
        return 0;
    } else if(total_remaining_h >= surface->h) {
        atlas->shelves[shelf_idx].width = surface->w;
        atlas->shelves[shelf_idx].height = surface->h;
        atlas->shelves[shelf_idx].count = 1;
        Kit_SetItemAllocation(
            item,
            surface,
            shelf_idx,
            0,
            0,
            total_reserved_h);
        return 0;
    }

    return 1; // Can't fit!
}

void Kit_CheckAtlasTextureSize(Kit_TextureAtlas *atlas, SDL_Texture *texture) {
    assert(atlas != NULL);
    assert(texture != NULL);

    // Check if texture size has changed, and clear content if it has.
    int texture_w;
    int texture_h;
    if(SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h) == 0) {
        atlas->w = texture_w;
        atlas->h = texture_h;
    }
}

int Kit_GetAtlasItems(const Kit_TextureAtlas *atlas, SDL_Rect *sources, SDL_Rect *targets, int limit) {
    assert(atlas != NULL);
    assert(limit >= 0);

    int max_count = min(atlas->cur_items, limit);
    for(int i = 0; i < max_count; i++) {
        Kit_TextureAtlasItem *item = &atlas->items[i];
        if(sources != NULL)
            memcpy(&sources[i], &item->source, sizeof(SDL_Rect));
        if(targets != NULL)
            memcpy(&targets[i], &item->target, sizeof(SDL_Rect));
    }
    return max_count;
}

int Kit_AddAtlasItem(Kit_TextureAtlas *atlas, SDL_Texture *texture, SDL_Surface *surface, const SDL_Rect *target) {
    assert(atlas != NULL);
    assert(surface != NULL);
    assert(target != NULL);

    // Make sure there is still room
    if(atlas->cur_items >= atlas->max_items)
        return -1;

    // Create a new item
    Kit_TextureAtlasItem item;
    memset(&item, 0, sizeof(Kit_TextureAtlasItem));
    memcpy(&item.target, target, sizeof(SDL_Rect));
    item.cur_shelf = -1;
    item.cur_slot = -1;

    // Allocate space for the new item
    if(Kit_FindFreeAtlasSlot(atlas, surface, &item) != 0) {
        return -1;
    }

    // And update texture with the surface
    SDL_UpdateTexture(texture, &item.source, surface->pixels, surface->pitch);

    // Room found, add item to the atlas
    memcpy(&atlas->items[atlas->cur_items++], &item, sizeof(Kit_TextureAtlasItem));
    return 0;
}

Kit_TextureAtlasItem *Kit_AddAtlasItemRaw(Kit_TextureAtlas *atlas, SDL_Surface *surface, const SDL_Rect *target) {

    assert(atlas != NULL);
    assert(surface != NULL);
    assert(target != NULL);

    // Make sure there is still room
    if(atlas->cur_items >= atlas->max_items)
        return NULL;

    // Create a new item
    Kit_TextureAtlasItem item;
    memset(&item, 0, sizeof(Kit_TextureAtlasItem));
    memcpy(&item.target, target, sizeof(SDL_Rect));
    item.cur_shelf = -1;
    item.cur_slot = -1;

    // Allocate space for the new item
    if(Kit_FindFreeAtlasSlot(atlas, surface, &item) != 0) {
        return NULL;
    }

    // Room found, add item to the atlas
    memcpy(&atlas->items[atlas->cur_items++], &item, sizeof(Kit_TextureAtlasItem));

    return &atlas->items[atlas->cur_items - 1];
}
