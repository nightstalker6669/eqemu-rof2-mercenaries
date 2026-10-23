#ifndef EQEMU_CLIENT_MERCENARY_TRANSFER_H
#define EQEMU_CLIENT_MERCENARY_TRANSFER_H
class Client;
class EQApplicationPacket;
namespace MercenaryTransfer {
// Actors come from the authenticated Client. Wire identities and prices never
// authorize a transfer; acceptance consumes a delivered, expiring server offer.
void Request(Client& client, const EQApplicationPacket& packet);
void Respond(Client& client, const EQApplicationPacket& packet);
}
#endif
