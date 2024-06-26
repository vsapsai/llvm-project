; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -passes=slp-vectorizer -S | FileCheck %s

target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

; Protect against a crash with scalable vector users

define i1 @crash(i32 %a, i32 %b) {
; CHECK-LABEL: @crash(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CONV_I446:%.*]] = sext i32 [[A:%.*]] to i64
; CHECK-NEXT:    [[CMP_I618870_NOT_NOT:%.*]] = icmp ult i64 0, [[CONV_I446]]
; CHECK-NEXT:    [[CONV_I401:%.*]] = sext i32 [[B:%.*]] to i64
; CHECK-NEXT:    [[CMP_I407876_NOT_NOT:%.*]] = icmp ult i64 0, [[CONV_I401]]
; CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 2 x i1> @llvm.aarch64.sve.whilelo.nxv2i1.i64(i64 0, i64 [[CONV_I401]])
; CHECK-NEXT:    [[R:%.*]] = select i1 [[CMP_I618870_NOT_NOT]], i1 [[CMP_I407876_NOT_NOT]], i1 false
; CHECK-NEXT:    ret i1 [[R]]
;
entry:
  %conv.i446 = sext i32 %a to i64
  %cmp.i618870.not.not = icmp ult i64 0, %conv.i446
  %conv.i401 = sext i32 %b to i64
  %cmp.i407876.not.not = icmp ult i64 0, %conv.i401
  %0 = tail call <vscale x 2 x i1> @llvm.aarch64.sve.whilelo.nxv2i1.i64(i64 0, i64 %conv.i401)
  %r = select i1 %cmp.i618870.not.not, i1 %cmp.i407876.not.not, i1 0
  ret i1 %r
}
