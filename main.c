#include <efi.h>
#include <efilib.h>

#include "main.h"
#include "fifo.h"
#include "debug.h"
#include "forth.h"

EFI_HANDLE GIH = NULL;
EFI_SYSTEM_TABLE *GST = NULL;

struct EventSlot *EventSlotHead = NULL;
uint32_t EventSlotNums = 0;

struct EventSlot *Event_Add(EFI_EVENT Event,
			    void (*CallBack)(struct EventSlot *EvtSlot,
					     EFI_STATUS Status),
			    VOID *Context)
{
	struct EventSlot *slot = NULL;
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 sizeof(struct EventSlot),
						 (void *)&slot);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	slot->Event = Event;
	slot->CallBack = CallBack;
	slot->Context = Context;
	slot->next = slot;
	if (EventSlotHead == NULL) {
		EventSlotHead = slot;
	}
	slot->next = EventSlotHead->next;
	EventSlotHead->next = slot;
	EventSlotNums += 1;
	return slot;
}

EFI_STATUS Event_Del(struct EventSlot *EvtSlot)
{
	if (EvtSlot == NULL) {
		return EFI_INVALID_PARAMETER;
	}
	if (EvtSlot == EventSlotHead) {
		EventSlotHead = EvtSlot->next;
	}
	struct EventSlot *PrevSlot;
	PrevSlot = EvtSlot;
	while (PrevSlot->next != EvtSlot) {
		PrevSlot = PrevSlot->next;
	}
	PrevSlot->next = EvtSlot->next;
	EFI_STATUS Status;
	Status = GST->BootServices->FreePool(EvtSlot);
	EventSlotNums -= 1;
	if (EventSlotNums == 0) {
		EventSlotHead = NULL;
	}
	return Status;
}

struct EventSlot *Event_Find(EFI_EVENT Event)
{
	UINTN Idx = 0;
	struct EventSlot *EvtSlot = EventSlotHead;
	while (Idx < EventSlotNums) {
		if (EvtSlot->Event == Event) {
			return EvtSlot;
		}
		EvtSlot = EvtSlot->next;
		Idx += 1;
	}
	return NULL;
}

struct fifo16 *early_wkey_fifo = NULL;
bool early_echo = false;

void cb_ConInWaitForKey(struct EventSlot *EvtSlot, EFI_STATUS Status)
{
	(void)EvtSlot;
	if (EFI_ERROR(Status)) {
		return;
	}
	EFI_INPUT_KEY Key;
	Status = GST->ConIn->ReadKeyStroke(GST->ConIn, &Key);
	if (EFI_ERROR(Status)) {
		return;
	}
	if (early_echo) {
		debug_putwc(Key.UnicodeChar);
	}
	if (fifo16_push(early_wkey_fifo, Key.UnicodeChar) == false) {
		debug_putws(L"EARLY WKEY FIFO IS FULL\r\n");
	}
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle,
			   EFI_SYSTEM_TABLE *SystemTable)
{
	GST = SystemTable;
	GIH = ImageHandle;
	GST->ConOut->OutputString(GST->ConOut, L"FORTH\r\n");
	early_wkey_fifo = fifo16_new(128);
	if (early_wkey_fifo == NULL) {
		debug_putws(L"ALLOC EARLY WKEY FIFO FAIL\r\n");
		return EFI_OUT_OF_RESOURCES;
	}
	if (Event_Add(GST->ConIn->WaitForKey, cb_ConInWaitForKey, NULL) ==
	    NULL) {
		debug_putws(L"ADD EVENT FAIL\r\n");
		return EFI_OUT_OF_RESOURCES;
	}
	forth_init();
	EFI_EVENT *WaitList = NULL;
	EFI_STATUS Status;
	struct EventSlot *EvtSlot = NULL;
	INTN Idx;
	UINTN UIdx;
	//debug_puthex(0x01234567);
	//debug_puthex(0x89ABCDEF);
	//debug_cr();
	// DISABLE WATCHDOG
	SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	while (1) {
		forth_run_all();
		if (WaitList != NULL) {
			GST->BootServices->FreePool(WaitList);
			WaitList = NULL;
		}
		Status = GST->BootServices->AllocatePool(
			EfiRuntimeServicesData,
			sizeof(EFI_EVENT) * EventSlotNums, (void *)&WaitList);
		if (EFI_ERROR(Status)) {
			debug_putws(L"ALLOT EVENT WAIT LIST FAIL\r\n");
			return EFI_OUT_OF_RESOURCES;
		}
		EvtSlot = EventSlotHead;
		Idx = 0;
		while (Idx < EventSlotNums) {
			WaitList[Idx] = EvtSlot->Event;
			EvtSlot = EvtSlot->next;
			Idx += 1;
		}
		UIdx = 0;
		Status = GST->BootServices->WaitForEvent(EventSlotNums,
							 WaitList, &UIdx);
		EvtSlot = Event_Find(WaitList[UIdx]);
		if (EvtSlot == NULL) {
			debug_putws(L"EVENT NOT IN LINKED LIST\r\n");
			return EFI_OUT_OF_RESOURCES;
		}
		if (EvtSlot->CallBack != NULL) {
			EvtSlot->CallBack(EvtSlot, Status);
		}
	}
}
