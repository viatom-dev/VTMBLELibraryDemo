//
//  VTMWBP02Header.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/18.
//

#ifndef VTMWBP02Header_h
#define VTMWBP02Header_h

#import <VTMBLELibrary/VTMWBP02BLESession.h>
#import <VTMBLELibrary/VTMWBP02Object.h>


#define LE_P2U16(p,u) do{u=0;u = (p)[0]|((p)[1]<<8);}while(0)
#define LE_P2U32(p,u) do{u=0;u = (p)[0]|((p)[1]<<8)|((p)[2]<<16)|((p)[3]<<24);}while(0)
#define BE_P2U16(p,u) do{u=0;u = ((p)[0]<<8)|((p)[1]);}while(0)
#define BE_P2U32(p,u) do{u=0;u = ((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|((p)[3]);}while(0)


#endif /* VTMWBP02Header_h */
