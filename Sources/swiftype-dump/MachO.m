#import "MachO.h"

NSDictionary<NSValue *, NSString *> *reverseSymbolLookUp(const struct mach_header *const mh) {
    // TODO: see fishhook to implement
    // https://github.com/facebook/fishhook/blob/aadc161ac3b80db07a9908851839a17ba63a9eb1/fishhook.c#L167
    assert(0 && "Not yet implemented");
    
    const struct load_command *lc = NULL;
    switch (mh->magic) {
        case MH_MAGIC_64:
            lc = (void *)mh + sizeof(struct mach_header_64);
            break;
        case MH_MAGIC:
            lc = (void *)mh + sizeof(struct mach_header);
            break;
        default:
            assert(0 && "unknown mach_header magic");
            return nil;
    }
    
    for (uint32_t cmd = 0; cmd < mh->ncmds; cmd++) {
        if (lc->cmd == LC_SYMTAB) {
            const struct symtab_command *sym_cmd = (const void *)lc;
            
            // sizeof(struct nlist_64) == 16
            // sizeof(struct nlist) == 12
            switch (mh->magic) {
                case MH_MAGIC_64: {
                    const struct nlist_64 *sym_table = (void *)mh + sym_cmd->symoff;
                    const char *str_table = (void *)mh + sym_cmd->stroff;
                    for (uint32_t sym_index = 0; sym_index < sym_cmd->nsyms; sym_index++) {
                        const struct nlist_64 *entry = sym_table + sym_index;
                        const char *name = str_table + entry->n_un.n_strx;
                        printf("%s [0x%" __UINT8_FMTx__ "]\n", name, entry->n_type);
                    }
                } break;
                case MH_MAGIC: {
                    const struct nlist *sym_table = (void *)mh + sym_cmd->symoff;
                    const char *str_table = (void *)mh + sym_cmd->stroff;
                    for (uint32_t sym_index = 0; sym_index < sym_cmd->nsyms; sym_index++) {
                        const struct nlist *entry = sym_table + sym_index;
                        const char *name = str_table + entry->n_un.n_strx;
                        printf("%s [0x%" __UINT8_FMTx__ "]\n", name, entry->n_type);
                    }
                } break;
                default:
                    assert(0 && "unknown mach_header magic");
                    return nil;
            }
        }
        lc = (void *)lc + lc->cmdsize;
    }
    
    return nil;
}

const struct mach_header *machHeaderForImage(const void *const image) {
    // TODO: how should we pick which slice to use?
    const uint32_t magic = *(const uint32_t *)image;
    switch (magic) {
        case MH_MAGIC: {
            return image;
        } break;
        case MH_MAGIC_64: {
            return image;
        } break;
        case FAT_MAGIC: {
            // TODO
            assert(0 && "FAT currently unsupported");
            return nil;
        } break;
        case FAT_MAGIC_64: {
            // TODO
            assert(0 && "FAT_64 currently unsupported");
            return nil;
        } break;
        default:
            assert(0 && "unknown file");
            return nil;
    }
}
