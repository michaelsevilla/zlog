#include "log_mapper.h"
#include <sstream>
#include "stripe_history.h"

std::string LogMapper::SlotToOid(int slot) const
{
    std::stringstream oid;
    oid << log_name_ << "." << slot;
    return oid.str();
}

void LogMapper::LatestObjectSet(std::vector<std::string>& objects,
    const StripeHistory& history) const
{
  const StripeHistory::Stripe stripe = history.LatestStripe();
  std::vector<std::string> result;
  for (int slot = 0; slot < stripe.width; slot++) {
    result.push_back(SlotToOid(slot));
  }
  objects.swap(result);
}

std::string LogMapper::FindObject(uint64_t position) const
{
  assert(!history_.Empty());
  const StripeHistory::Stripe stripe = history_.FindStripe(position);
  int slot = position % stripe.width;
  return SlotToOid(slot);
}
