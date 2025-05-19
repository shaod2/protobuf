use super::sys::message as msg_sys;
use super::{Arena, AssociatedMiniTable, MiniTable};
use core::marker::PhantomData;

pub type RawMessage = super::sys::message::RawMessage;

/// # Safety
/// - `m` and `mini_table` arguments must be valid to deref
/// - `mini_table` must be the MiniTable associated with `m`
pub unsafe fn deep_clone(
    m: RawMessage,
    mini_table: *const MiniTable,
    arena: &Arena,
) -> Option<RawMessage> {
    unsafe {
        // SAFETY:
        // - Caller ensures `m` and `mini_table` arguments must be valid to deref
        // -  Caller ensures `mini_table` must be the MiniTable associated with `m`
        msg_sys::upb_Message_DeepClone(m, mini_table, arena.raw())
    }
}

pub struct MessagePtr<T: AssociatedMiniTable> {
    raw: RawMessage,
    _phantom: PhantomData<T>,
}

impl<T: AssociatedMiniTable> MessagePtr<T> {
    pub fn new(arena: &Arena) -> Self {
        // SAFETY:
        // - `T::mini_table` is a valid MiniTable
        // - `arena.raw()` is a valid UPB arena
        let raw = unsafe { msg_sys::upb_Message_New(T::mini_table(), arena.raw()) }
            .expect("upb_Message_New should never fail");
        Self { raw, _phantom: PhantomData }
    }

    pub fn clone(&self, arena: &Arena) -> Self {
        // SAFETY:
        // - `T::mini_table` is a valid MiniTable associated with `self.raw`.
        // - `arena.raw()` is a valid UPB arena
        let raw = unsafe { msg_sys::upb_Message_DeepClone(self.raw, T::mini_table(), arena.raw()) }
            .expect("upb_Message_DeepClone should never fail");
        Self { raw, _phantom: PhantomData }
    }
}
